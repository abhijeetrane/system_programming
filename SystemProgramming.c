#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Structure matching the kernel's sysinfo structure for raw assembly syscall
struct sysinfo_raw {
    long uptime;             /* Seconds since boot */
    unsigned long loads[3];  /* 1, 5, and 15 minute load averages */
    unsigned long totalram;  /* Total usable main memory size */
    unsigned long freeram;   /* Available memory size */
    unsigned long sharedram; /* Amount of shared memory */
    unsigned long bufferram; /* Memory used by buffers */
    unsigned long totalswap; /* Total swap space size */
    unsigned long freeswap;  /* Swap space still available */
    unsigned short procs;    /* Number of current processes */
    unsigned short pad;
    unsigned long totalhigh; /* Total high memory size */
    unsigned long freehigh;  /* Available high memory size */
    unsigned int mem_unit;   /* Memory unit size in bytes */
    char _f[20-2*sizeof(long)-sizeof(int)]; /* Padding */
};

/* 1. Reads CPU Registers using Inline Assembly */
void print_cpu_registers(void) {
    uint64_t rax_val, rbx_val, rcx_val, rdx_val, rsp_val; // @suppress("Type cannot be resolved")

    // Use inline asm to copy live register values into C variables
    __asm__ __volatile__ (
        "mov %%rax, %0\n\t"
        "mov %%rbx, %1\n\t"
        "mov %%rcx, %2\n\t"
        "mov %%rdx, %3\n\t"
        "mov %%rsp, %4\n\t"
        : "=r"(rax_val), "=r"(rbx_val), "=r"(rcx_val), "=r"(rdx_val), "=r"(rsp_val)
        :
        : "memory"
    );

    printf("--- CPU REGISTERS (Live Snapshots) ---\n");
    printf("RAX: 0x%016LX\n", (unsigned long long)rax_val);
    printf("RBX: 0x%016LX\n", (unsigned long long)rbx_val);
    printf("RCX: 0x%016LX\n", (unsigned long long)rcx_val);
    printf("RDX: 0x%016LX\n", (unsigned long long)rdx_val);
    printf("RSP: 0x%016LX (Stack Pointer)\n\n", (unsigned long long)rsp_val);
}

/* 2. Uses CPUID assembly instruction to get Processor Brand String */
void print_cpu_info(void) {
    char vendor[13];
    vendor[12] = '\0';

    // Call cpuid leaf 0: EBX, EDX, ECX contain vendor string (e.g., "GenuineIntel")
    __asm__ __volatile__ (
        "mov $0, %%eax\n\t"
        "cpuid\n\t"
        "mov %%ebx, %0\n\t"
        "mov %%edx, %1\n\t"
        "mov %%ecx, %2\n\t"
        : "=m"(vendor[0]), "=m"(vendor[4]), "=m"(vendor[8])
        :
        : "eax", "ebx", "ecx", "edx"
    );

    printf("--- CPU IDENTIFICATION ---\n");
    printf("Processor Vendor: %s\n\n", vendor);
}

/* 3. Uses Raw x86_64 'syscall' Assembly to get Memory Capacity */
void print_memory_capacity(void) {
    struct sysinfo_raw info;
    long sys_ret;

    // Syscall number 99 on x86_64 Linux is sysinfo(struct sysinfo *info)
    // RAX = 99 (sys_sysinfo)
    // RDI = pointer to buffer (&info)
    __asm__ __volatile__ (
        "mov $99, %%rax\n\t"    // sys_sysinfo syscall ID
        "mov %1, %%rdi\n\t"     // Pass address of 'info' struct to RDI
        "syscall\n\t"           // Invoke Linux Kernel interrupt
        "mov %%rax, %0\n\t"     // Store return value
        : "=r"(sys_ret)
        : "r"(&info)
        : "rax", "rdi", "rcx", "r11", "memory"
    );

    if (sys_ret == 0) {
        unsigned long unit = info.mem_unit ? info.mem_unit : 1;
        unsigned long long total_bytes = (unsigned long long)info.totalram * unit;
        unsigned long long free_bytes  = (unsigned long long)info.freeram * unit;
        unsigned long long used_bytes  = total_bytes - free_bytes;

        printf("--- MEMORY CAPACITY & USAGE (via Assembly Syscall) ---\n");
        printf("Total Physical RAM : %.2f GB (%llu Bytes)\n", total_bytes / (1024.0 * 1024 * 1024), total_bytes);
        printf("Used Physical RAM  : %.2f GB (%llu Bytes)\n", used_bytes / (1024.0 * 1024 * 1024), used_bytes);
        printf("Free Physical RAM  : %.2f GB (%llu Bytes)\n", free_bytes / (1024.0 * 1024 * 1024), free_bytes);
    } else {
        printf("Failed to retrieve system memory info via syscall.\n");
    }
}

int main(void) {
    print_cpu_info();
    print_cpu_registers();
    print_memory_capacity();
    return 0;
}
