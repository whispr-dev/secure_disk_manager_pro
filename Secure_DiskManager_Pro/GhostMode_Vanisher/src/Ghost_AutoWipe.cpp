#include "Ghost_AutoWipe.h"
#include "SecureDelete.h"
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>

Ghost_AutoWipe::Ghost_AutoWipe(const std::string& path, int delaySeconds)
    : watchPath(path), delay(delaySeconds), armed(false) {}

void Ghost_AutoWipe::arm() {
    armed = true;
    std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::seconds(delay));
        if (armed) {
            std::cout << "[Ghost_AutoWipe] Triggering secure wipe of: " << watchPath << "\n";
            SecureDelete::shredPath(watchPath);
        }
    }).detach();
}

void Ghost_AutoWipe::abort() {
    armed = false;
    std::cout << "[Ghost_AutoWipe] Auto-wipe aborted.\n";
}
