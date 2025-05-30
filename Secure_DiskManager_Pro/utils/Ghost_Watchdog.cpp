#include "GhostWatchdog.h"
#include <iostream>
#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#include <TlHelp32.h>
#endif

GhostWatchdog::GhostWatchdog() : running(false) {}

GhostWatchdog::~GhostWatchdog() {
    stop();
}

void GhostWatchdog::addWatchedProcess(const std::string& processName) {
    watchedProcesses.push_back(processName);
}

void GhostWatchdog::start() {
    running = true;
    monitorThread = std::thread(&GhostWatchdog::monitorLoop, this);
}

void GhostWatchdog::stop() {
    running = false;
    if (monitorThread.joinable())
        monitorThread.join();
}

void GhostWatchdog::monitorLoop() {
    while (running) {
        for (const auto& proc : watchedProcesses) {
            if (!isProcessRunning(proc)) {
                std::cerr << "[!] Watchdog alert: " << proc << " not running!\n";
                // In production: call kill switch or attempt restart
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

bool GhostWatchdog::isProcessRunning(const std::string& name) {
#ifdef _WIN32
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnap, &pe)) {
        do {
            if (name == pe.szExeFile) {
                CloseHandle(hSnap);
                return true;
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
#endif
    return false;
}
