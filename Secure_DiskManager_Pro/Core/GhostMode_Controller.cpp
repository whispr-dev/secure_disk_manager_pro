#include "GhostMode_Controller.h"
#include "mac_spoofer.h"
#include "gpg_wrapper.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

GhostMode_Controller::GhostMode_Controller()
    : initialized(false), activePersona("") {}

bool GhostMode_Controller::Initialize() {
    std::cout << "[GhostMode] Initializing controller...\n";
    initialized = true;
    return true;
}

void GhostMode_Controller::Shutdown() {
    std::cout << "[GhostMode] Shutting down.\n";
    initialized = false;
}

bool GhostMode_Controller::LoadPersona(const std::string& personaName) {
    if (!initialized) return false;

    std::string personaPath = "./identities/" + personaName;
    if (!fs::exists(personaPath)) return false;

    activePersona = personaName;

    if (!LoadMACConfig(personaPath)) {
        std::cerr << "[GhostMode] Failed to spoof MAC.\n";
        return false;
    }

    if (!LoadGPGKey(personaPath)) {
        std::cerr << "[GhostMode] Failed to load GPG identity.\n";
        return false;
    }

    std::cout << "[GhostMode] Loaded persona: " << personaName << "\n";
    return true;
}

bool GhostMode_Controller::SavePersonaState() {
    if (!initialized || activePersona.empty()) return false;
    std::cout << "[GhostMode] Saving persona state: " << activePersona << "\n";
    return true;
}

std::vector<std::string> GhostMode_Controller::ListAvailablePersonas() {
    std::vector<std::string> personas;
    std::string base = "./identities";
    for (const auto& entry : fs::directory_iterator(base)) {
        if (entry.is_directory()) {
            personas.push_back(entry.path().filename().string());
        }
    }
    return personas;
}

bool GhostMode_Controller::ApplyMACSpoofing() {
    if (activePersona.empty()) return false;
    return mac_spoofer::SpoofMACForPersona("./identities/" + activePersona);
}

bool GhostMode_Controller::ApplyGPGIdentity() {
    if (activePersona.empty()) return false;
    return gpg_wrapper::LoadGPGForPersona("./identities/" + activePersona);
}

void GhostMode_Controller::EnableStealthMode() {
    std::cout << "[GhostMode] Stealth mode enabled.\n";
    // add more triggers here
}

void GhostMode_Controller::DisableStealthMode() {
    std::cout << "[GhostMode] Stealth mode disabled.\n";
    // add more teardown
}

std::string GhostMode_Controller::GetCurrentPersona() const {
    return activePersona;
}

bool GhostMode_Controller::IsInitialized() const {
    return initialized;
}

bool GhostMode_Controller::LoadMACConfig(const std::string& path) {
    return mac_spoofer::SpoofMACForPersona(path);
}

bool GhostMode_Controller::LoadGPGKey(const std::string& path) {
    return gpg_wrapper::LoadGPGForPersona(path);
}
