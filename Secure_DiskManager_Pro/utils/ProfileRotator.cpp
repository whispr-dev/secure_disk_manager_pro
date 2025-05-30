#include "profile_rotator.h"
#include "mac_spoofer.h"
#include "gpg_wrapper.h"
#include <fstream>
#include <sstream>
#include <iostream>

ProfileRotator::ProfileRotator(const std::string& iface) : interface(iface) {}

bool ProfileRotator::loadPersonas(const std::string& configFile) {
    std::ifstream infile(configFile);
    if (!infile.is_open()) {
        std::cerr << "[!] Failed to open persona config: " << configFile << "\n";
        return false;
    }

    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        Persona p;
        if (!(iss >> p.name >> p.macAddress >> p.gpgKeyID >> p.gpgKeyFile)) {
            std::cerr << "[!] Malformed line in persona config: " << line << "\n";
            continue;
        }
        personas.push_back(p);
    }

    return !personas.empty();
}

bool ProfileRotator::rotateToNext() {
    if (personas.empty()) {
        std::cerr << "[!] No personas loaded.\n";
        return false;
    }

    currentIndex = (currentIndex + 1) % personas.size();
    return applyPersona(personas[currentIndex]);
}

const Persona& ProfileRotator::getCurrentPersona() const {
    return personas[currentIndex];
}

bool ProfileRotator::applyPersona(const Persona& p) {
    std::cout << "[*] Applying persona: " << p.name << "\n";

    if (!MacSpoofer::spoofMac(interface, p.macAddress)) {
        std::cerr << "[!] Failed to spoof MAC: " << p.macAddress << "\n";
        return false;
    }

    if (!GpgWrapper::importKey(p.gpgKeyFile)) {
        std::cerr << "[!] Failed to import GPG key: " << p.gpgKeyFile << "\n";
        return false;
    }

    if (!GpgWrapper::keyExists(p.gpgKeyID)) {
        std::cerr << "[!] GPG key " << p.gpgKeyID << " does not exist.\n";
        return false;
    }

    std::cout << "[+] Persona " << p.name << " applied successfully.\n";
    return true;
}
