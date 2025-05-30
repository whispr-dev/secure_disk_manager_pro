#ifndef GHOST_WATCHDOG_H
#define GHOST_WATCHDOG_H

#include <string>
#include <vector>
#include <thread>
#include <atomic>

class GhostWatchdog {
public:
    GhostWatchdog();
    ~GhostWatchdog();

    void addWatchedProcess(const std::string& processName);
    void start();
    void stop();

private:
    void monitorLoop();
    bool isProcessRunning(const std::string& name);

    std::vector<std::string> watchedProcesses;
    std::atomic<bool> running;
    std::thread monitorThread;
};

#endif // GHOST_WATCHDOG_H
