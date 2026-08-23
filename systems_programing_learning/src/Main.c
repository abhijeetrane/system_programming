#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <windows.h>
#include <psapi.h>

/* Structure matching the kernel's sysinfo structure for raw assembly syscall
 *
 * Author: Abhijeet Rane and Artificial Intelligence
 *
 */

/********************************************************************************************************/
//Total Free Available Memory in Windows x86-64
/********************************************************************************************************/
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

//Total Free Available Memory in Windows x86-64

int getTotalFreeAvailableMemoryinWindowsx86_64(){
	MEMORYSTATUSEX statex;
	    statex.dwLength = sizeof(statex);

	    if (get_memory_status_asm(&statex)) {
	        // ullAvailPhys holds the free physical RAM available to processes
	        double free_gib = (double)statex.ullAvailPhys / (1024.0 * 1024.0 * 1024.0);
	        //double total_gib = (double)statex.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);

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
/********************************************************************************************************/

/********************************************************************************************************/
//Memory used by buffers in Windows x86-64
/********************************************************************************************************/
// Automatically link against the Psapi library (required for GetProcessMemoryInfo)
//#pragma comment(lib, "psapi.lib")



int getMemoryUsedByBuffersInWindowsx86_64(){
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

/********************************************************************************************************/
//Total Shared Memory Windowsx86_64
/********************************************************************************************************/

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

int getTotalSharedMemoryWindowsx86_64(void) {
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


int main(void) {
	    printf("********************************************************************************************************\n");
	    printf("Structure matching the kernel's sysinfo structure for raw assembly syscall");
	    printf("\n********************************************************************************************************\n");
	    int returnValue1 = getTotalFreeAvailableMemoryinWindowsx86_64();

	    int returnValue2 = 0;

	    if (returnValue1 == 0){
	    	printf("********************************************************************************************************\n");
	    	returnValue2 =  getMemoryUsedByBuffersInWindowsx86_64();
	    	printf("********************************************************************************************************\n");

	    }

	    int returnValue3 = 0;

	    if (returnValue2 == 0){
	    	   printf("********************************************************************************************************\n");
	    	   int returnValue3 =  getTotalSharedMemoryWindowsx86_64();
	    	   printf("********************************************************************************************************\n");
	    	   return returnValue3;
	    }

	    printf("********************************************************************************************************\n");
        return returnValue3;
}
