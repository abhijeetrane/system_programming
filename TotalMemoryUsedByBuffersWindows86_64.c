#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <psapi.h>

// Automatically link against the Psapi library (required for GetProcessMemoryInfo)
#pragma comment(lib, "psapi.lib")

int main(void) {
    // Define sizes for multiple memory buffers (e.g., 5 MB, 10 MB, and 2 MB)
    size_t buffer_sizes[] = {
        5 * 1024 * 1024,
        10 * 1024 * 1024,
        2 * 1024 * 1024
    };
    int num_buffers = sizeof(buffer_sizes) / sizeof(buffer_sizes[0]);
    void* buffers[3];

    printf("--- Allocating Buffers ---\n");
    for (int i = 0; i < num_buffers; i++) {
        buffers[i] = malloc(buffer_sizes[i]);
        if (buffers[i] == NULL) {
            fprintf(stderr, "Failed to allocate memory for buffer %d\n", i);
            return 1;
        }
        printf("Buffer %d allocated: %zu bytes (%.2f MB)\n",
               i, buffer_sizes[i], (double)buffer_sizes[i] / (1024.0 * 1024.0));
    }

    // Query Windows Process Memory Statistics
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        printf("\n--- Memory Usage Metrics ---\n");
        printf("Working Set Size (Physical RAM used): %zu bytes (%.2f MB)\n",
               pmc.WorkingSetSize, (double)pmc.WorkingSetSize / (1024.0 * 1024.0));
        printf("Commit Size (Total Allocated Virtual): %zu bytes (%.2f MB)\n",
               pmc.PagefileUsage, (double)pmc.PagefileUsage / (1024.0 * 1024.0));
    } else {
        fprintf(stderr, "Failed to retrieve process memory info. Error code: %lu\n", GetLastError());
    }

    // Clean up allocated memory
    for (int i = 0; i < num_buffers; i++) {
        free(buffers[i]);
    }

    return 0;
}
