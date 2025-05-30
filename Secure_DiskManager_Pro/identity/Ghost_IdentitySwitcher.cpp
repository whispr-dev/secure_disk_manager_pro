#include "Ghost_Identity_Manager.h"
#include <filesystem>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

Ghost_Identity_Manager::Ghost_Identity_Manager() : identitiesRoot("identities/"), activeIdentity("") {}

void Ghost_Identity_Manager::loadIdentities(const std::string& directory) {
    identitiesRoot = directory;
    identityPaths.clear();

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();
            identityPaths[name] = entry.path().string();
        }
    }
}

std::vector<std::string> Ghost_Identity_Manager::listIdentities() const {
    std::vector<std::string> names;
    for (const auto& [name, path] : identityPaths) {
        names.push_back(name);
    }
    return names;
}

bool Ghost_Identity_Manager::switchToIdentity(const std::string& identityName) {
    if (!identityPaths.contains(identityName)) {
        std::cerr << "[X] Identity not found: " << identityName << "\n";
        return false;
    }

    std::string basePath = identityPaths.at(identityName);
    if (!applyMAC(basePath + "/mac.txt") ||
        !loadGPGProfile(basePath + "/gpg-key.asc") ||
        !loadBrowserProfile(basePath + "/firefox-profile")) {
        return false;
    }

    activeIdentity = identityName;
    std::cout << "[✓] Switched to identity: " << identityName << "\n";
    return true;
}

std::string Ghost_Identity_Manager::getCurrentIdentity() const {
    return activeIdentity;
}

bool Ghost_Identity_Manager::applyMAC(const std::string& macFilePath) {
    std::ifstream file(macFilePath);
    if (!file) {
        std::cerr << "[!] MAC file not found: " << macFilePath << "\n";
        return false;
    }

    std::string mac;
    std::getline(file, mac);
    std::string cmd = "mac_spoofer " + mac;
    return std::system(cmd.c_str()) == 0;
}

bool Ghost_Identity_Manager::loadGPGProfile(const std::string& gpgKeyPath) {
    std::string cmd = "gpg --import \"" + gpgKeyPath + "\"";
    return std::system(cmd.c_str()) == 0;
}

bool Ghost_Identity_Manager::loadBrowserProfile(const std::string& firefoxProfilePath) {
    std::string cmd = "cp -r \"" + firefoxProfilePath + "\" ~/.mozilla/firefox/default-release/";
    return std::system(cmd.c_str()) == 0;
}
