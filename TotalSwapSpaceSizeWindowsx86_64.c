#include <stdio.h>
#include <windows.h>
#include <psapi.h>

// Automatically link against the Psapi library
#pragma comment(lib, "psapi.lib")

int main(void) {
    // 1. Get system-wide Memory and Page File Status
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);

    if (!GlobalMemoryStatusEx(&memStatus)) {
        fprintf(stderr, "Failed to retrieve memory status. Error: %lu\n", GetLastError());
        return 1;
    }

    // 2. Get detailed system-wide page file performance info
    PERFORMANCE_INFORMATION perfInfo;
    perfInfo.cb = sizeof(perfInfo);

    if (!GetPerformanceInfo(&perfInfo, sizeof(perfInfo))) {
        fprintf(stderr, "Failed to retrieve performance info. Error: %lu\n", GetLastError());
        return 1;
    }

    // Calculations based on Windows API metrics
    // ullTotalPageFile: Total system commit limit (Physical RAM + Page File size)
    DWORDLONG totalCommitLimit = memStatus.ullTotalPageFile;
    DWORDLONG totalRAM = memStatus.ullTotalPhys;
    DWORDLONG estimatedPageFileSize = (totalCommitLimit > totalRAM) ? (totalCommitLimit - totalRAM) : 0;

    // Page file commitments from GetPerformanceInfo
    size_t pageSize = perfInfo.PageSize;
    size_t totalCommitPages = perfInfo.CommitTotal;
    size_t limitCommitPages = perfInfo.CommitLimit;
    size_t commitBytesUsed = totalCommitPages * pageSize;
    size_t commitBytesLimit = limitCommitPages * pageSize;

    printf("=== Windows System Memory & Swap Info (x86-64) ===\n\n");

    printf("--- Physical Memory ---\n");
    printf("Total RAM:            %llu bytes (%.2f GB)\n\n",
           totalRAM, (double)totalRAM / (1024.0 * 1024.0 * 1024.0));

    printf("--- Swap Space (Paging File) ---\n");
    printf("Total Commit Limit:   %llu bytes (%.2f GB)\n",
           totalCommitLimit, (double)totalCommitLimit / (1024.0 * 1024.0 * 1024.0));
    printf("Estimated Swap Size:  %llu bytes (%.2f GB)\n\n",
           estimatedPageFileSize, (double)estimatedPageFileSize / (1024.0 * 1024.0 * 1024.0));

    printf("--- System Commit Status ---\n");
    printf("Committed Memory:     %zu bytes (%.2f GB)\n",
           commitBytesUsed, (double)commitBytesUsed / (1024.0 * 1024.0 * 1024.0));
    printf("Commit Limit:         %zu bytes (%.2f GB)\n",
           commitBytesLimit, (double)commitBytesLimit / (1024.0 * 1024.0 * 1024.0));

    return 0;
}
