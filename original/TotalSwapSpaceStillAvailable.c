#include <stdio.h>
#include <windows.h>

int main(void) {
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);

    // Retrieve system memory status
    if (!GlobalMemoryStatusEx(&memStatus)) {
        fprintf(stderr, "Failed to retrieve memory status. Error code: %lu\n", GetLastError());
        return 1;
    }

    // Windows API Definitions:
    // ullAvailPageFile: Current available commit capacity (Available RAM + Available Pagefile space)
    // ullAvailPhys:     Available physical RAM
    DWORDLONG totalAvailCommit = memStatus.ullAvailPageFile;
    DWORDLONG availRAM         = memStatus.ullAvailPhys;

    // Available pagefile-only capacity (if commit limit exceeds physical RAM)
    DWORDLONG availPagefileOnly = (totalAvailCommit > availRAM) ? (totalAvailCommit - availRAM) : 0;

    printf("=== Windows Available Swap & Commit Capacity (x86-64) ===\n\n");

    printf("Available Physical RAM:       %llu bytes (%.2f GB)\n",
           availRAM, (double)availRAM / (1024.0 * 1024.0 * 1024.0));

    printf("Total Available Commit Space: %llu bytes (%.2f GB)\n",
           totalAvailCommit, (double)totalAvailCommit / (1024.0 * 1024.0 * 1024.0));

    printf("Unused Pagefile Space (Est.): %llu bytes (%.2f GB)\n",
           availPagefileOnly, (double)availPagefileOnly / (1024.0 * 1024.0 * 1024.0));

    return 0;
}
