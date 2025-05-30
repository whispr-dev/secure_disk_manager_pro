#ifndef GHOST_FS_MONITOR_H
#define GHOST_FS_MONITOR_H

#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>

class Ghost_FS_Monitor {
public:
    using ChangeCallback = std::function<void(const std::string& path, const std::string& action)>;

    Ghost_FS_Monitor();
    ~Ghost_FS_Monitor();

    void watchDirectory(const std::string& path, ChangeCallback callback);
    void stopWatching();

private:
    void monitorLoop();
    std::string watchedPath;
    ChangeCallback changeCallback;
    std::thread monitorThread;
    std::atomic<bool> watching;

    void platformMonitor(); // OS-specific watcher
};

#endif // GHOST_FS_MONITOR_H
