#include "Ghost_FS_Monitor.h"
#include <iostream>
#include <chrono>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

Ghost_FS_Monitor::Ghost_FS_Monitor() : watching(false) {}

Ghost_FS_Monitor::~Ghost_FS_Monitor() {
    stopWatching();
}

void Ghost_FS_Monitor::watchDirectory(const std::string& path, ChangeCallback callback) {
    if (watching) return;
    watchedPath = path;
    changeCallback = callback;
    watching = true;
    monitorThread = std::thread(&Ghost_FS_Monitor::monitorLoop, this);
}

void Ghost_FS_Monitor::stopWatching() {
    watching = false;
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
}

void Ghost_FS_Monitor::monitorLoop() {
    std::unordered_map<std::string, fs::file_time_type> files;

    while (watching) {
        for (const auto& entry : fs::recursive_directory_iterator(watchedPath)) {
            const auto path = entry.path().string();
            auto lastWriteTime = fs::last_write_time(entry);

            if (!files.contains(path)) {
                files[path] = lastWriteTime;
                changeCallback(path, "created");
            } else if (files[path] != lastWriteTime) {
                files[path] = lastWriteTime;
                changeCallback(path, "modified");
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}
