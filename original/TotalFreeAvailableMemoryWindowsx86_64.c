#include <stdio.h>
#include <windows.h>

/*
 * Call GlobalMemoryStatusEx via x86_64 Microsoft ABI assembly.
 * Win64 ABI rules:
 * - 1st argument (lpBuffer) goes into RCX.
 * - Allocates 32 bytes shadow space + 8 bytes for 16-byte stack alignment.
 */
BOOL get_memory_status_asm(MEMORYSTATUSEX *pStat) {
    BOOL result;

    __asm__ __volatile__ (
        "subq $40, %%rsp\n\t"            // Shadow space (32 bytes) + alignment (8 bytes)
        "movq %1, %%rcx\n\t"             // Pass pointer to MEMORYSTATUSEX in RCX
        "call *%2\n\t"                   // Call GlobalMemoryStatusEx function pointer
        "addq $40, %%rsp\n\t"            // Restore stack frame
        "movl %%eax, %0\n\t"             // Store BOOL return value from EAX
        : "=r" (result)
        : "r" (pStat), "r" (GlobalMemoryStatusEx)
        : "rcx", "rdx", "r8", "r9", "r10", "r11", "memory"
    );

    return result;
}

int main(void) {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);

    if (get_memory_status_asm(&statex)) {
        // ullAvailPhys holds the free physical RAM available to processes
        double free_gib = (double)statex.ullAvailPhys / (1024.0 * 1024.0 * 1024.0);
        double total_gib = (double)statex.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);

        printf("Physical Memory Status (Windows x86_64):\n");
        printf("  Free Available RAM: %llu Bytes\n", statex.ullAvailPhys);
        printf("  Free Available RAM: %.2f GiB\n", free_gib);
        printf("  Memory Load:        %lu%%\n", statex.dwMemoryLoad);
    } else {
        printf("Failed to query memory status. Error code: %lu\n", GetLastError());
        return 1;
    }

    return 0;
}
