#include <stdio.h>
#include <windows.h>

/*
 * Call GlobalMemoryStatusEx via x86_64 Microsoft ABI assembly.
 * Win64 ABI rules:
 * - 1st argument (lpBuffer) goes into RCX.
 * - Must allocate 32 bytes of shadow space on the stack before calling.
 * - Stack must be 16-byte aligned before the call.
 */
BOOL call_GetMemoryStatus_asm(MEMORYSTATUSEX *pStat) {
    BOOL result;

    __asm__ __volatile__ (
        "subq $40, %%rsp\n\t"            // 32 bytes shadow space + 8 bytes alignment
        "movq %1, %%rcx\n\t"             // 1st argument (pStat) into RCX
        "call *%2\n\t"                   // Call GlobalMemoryStatusEx pointer
        "addq $40, %%rsp\n\t"            // Restore stack pointer
        "movl %%eax, %0\n\t"             // Return value (BOOL) in EAX
        : "=r" (result)
        : "r" (pStat), "r" (GlobalMemoryStatusEx)
        : "rcx", "rdx", "r8", "r9", "r10", "r11", "memory"
    );

    return result;
}

int main(void) {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);

    if (call_GetMemoryStatus_asm(&statex)) {
        double total_gib = (double)statex.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);

        printf("Total Usable Main Memory (Windows x86_64):\n");
        printf("  Bytes: %llu B\n", statex.ullTotalPhys);
        printf("  GiB:   %.2f GiB\n", total_gib);
    } else {
        printf("Failed to query memory status. Error code: %lu\n", GetLastError());
    }

    return 0;
}
