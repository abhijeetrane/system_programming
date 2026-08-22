#include <stdio.h>
#include <windows.h>
#include <psapi.h>

/*
 * Issues the call to K32GetProcessMemoryInfo using x86_64 Microsoft ABI assembly.
 * Win64 ABI rules:
 * - 1st arg (Process handle) -> RCX
 * - 2nd arg (PPROCESS_MEMORY_COUNTERS) -> RDX
 * - 3rd arg (Size of structure) -> R8
 * - Requires 32 bytes shadow space on stack + alignment
 */
BOOL get_process_memory_info_asm(HANDLE hProcess, PROCESS_MEMORY_COUNTERS_EX *pCounters, DWORD cb) {
    BOOL result;

    __asm__ __volatile__ (
        "subq $40, %%rsp\n\t"            // Reserve 32-byte shadow space + 8-byte stack align
        "movq %1, %%rcx\n\t"             // 1st arg: Process handle into RCX
        "movq %2, %%rdx\n\t"             // 2nd arg: Pointer to counters into RDX
        "movl %3, %%r8d\n\t"             // 3rd arg: Size of counters into R8D
        "call *%4\n\t"                   // Call K32GetProcessMemoryInfo
        "addq $40, %%rsp\n\t"            // Restore stack frame
        "movl %%eax, %0\n\t"             // Store BOOL return value from EAX
        : "=r" (result)
        : "r" (hProcess), "r" (pCounters), "r" (cb), "r" (K32GetProcessMemoryInfo)
        : "rcx", "rdx", "r8", "r9", "r10", "r11", "memory"
    );

    return result;
}

int main(void) {
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX pmc = {0};

    if (get_process_memory_info_asm(hProcess, &pmc, sizeof(pmc))) {
        // SharedCommit represents committed memory shared with other processes/sections
        double shared_mb = (double)pmc.PagefileUsage / (1024.0 * 1024.0);
        double working_set_mb = (double)pmc.WorkingSetSize / (1024.0 * 1024.0);

        printf("Process Shared Memory Status or Page file usage(Windows x86_64):\n");
        printf("  Shared Commit Memory or Page file usage: %llu Bytes (%.2f MB)\n", (unsigned long long)pmc.PagefileUsage, shared_mb);
        printf("  Total Working Set:    %llu Bytes (%.2f MB)\n", (unsigned long long)pmc.WorkingSetSize, working_set_mb);
    } else {
        printf("Failed to query process memory info. Error code: %lu\n", GetLastError());
        return 1;
    }

    return 0;
}
