#include "IdentitySwitcher.h"
#include "GhostUtils.h"
#include <filesystem>
#include <iostream>
#include <cstdlib>

IdentitySwitcher::IdentitySwitcher(const std::string& identityBasePath)
    : basePath(identityBasePath) {}

bool IdentitySwitcher::switchToIdentity(const std::string& identityName) {
    std::string targetPath = basePath + "/" + identityName;
    if (!std::filesystem::exists(targetPath)) {
        std::cerr << "[IdentitySwitcher] Error: Identity not found at " << targetPath << std::endl;
        return false;
    }

    std::string currentLink = basePath + "/active";
    if (std::filesystem::is_symlink(currentLink)) {
        std::filesystem::remove(currentLink);
    }

    try {
        std::filesystem::create_symlink(targetPath, currentLink);
        std::cout << "[IdentitySwitcher] Active identity switched to: " << identityName << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[IdentitySwitcher] Failed to switch identity: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> IdentitySwitcher::listIdentities() const {
    std::vector<std::string> names;
    for (const auto& dir : std::filesystem::directory_iterator(basePath)) {
        if (dir.is_directory() && dir.path().filename() != "active") {
            names.push_back(dir.path().filename().string());
        }
    }
    return names;
}
