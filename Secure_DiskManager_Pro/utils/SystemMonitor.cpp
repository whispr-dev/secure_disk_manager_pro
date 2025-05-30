// System Monitor
#include "SystemMonitor.h"
#include <windows.h>
#include <psapi.h>
#include <iostream>

static ULONGLONG lastIdleTime = 0, lastKernelTime = 0, lastUserTime = 0;

double SystemMonitor::getCpuUsage() {
    FILETIME idle, kernel, user;
    if (!GetSystemTimes(&idle, &kernel, &user)) return -1.0;

    ULONGLONG idleTime = *(ULONGLONG*)&idle;
    ULONGLONG kernelTime = *(ULONGLONG*)&kernel;
    ULONGLONG userTime = *(ULONGLONG*)&user;

    ULONGLONG deltaIdle = idleTime - lastIdleTime;
    ULONGLONG deltaKernel = kernelTime - lastKernelTime;
    ULONGLONG deltaUser = userTime - lastUserTime;

    lastIdleTime = idleTime;
    lastKernelTime = kernelTime;
    lastUserTime = userTime;

    ULONGLONG total = deltaKernel + deltaUser;
    return (total > 0) ? (100.0 - (100.0 * deltaIdle / total)) : -1.0;
}

void SystemMonitor::getMemoryStats(unsigned long long& total, unsigned long long& available) {
    MEMORYSTATUSEX mem = {};
    mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem)) {
        total = mem.ullTotalPhys;
        available = mem.ullAvailPhys;
    } else {
        total = available = 0;
    }
}
