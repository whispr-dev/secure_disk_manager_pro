#include "Persona_Switcher.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

Persona_Switcher::Persona_Switcher() : personaRoot("personas/") {}

void Persona_Switcher::setPersonaRoot(const std::string& rootPath) {
    personaRoot = rootPath;
}

std::vector<std::string> Persona_Switcher::listPersonas() {
    std::vector<std::string> personas;
    for (const auto& dir : fs::directory_iterator(personaRoot)) {
        if (dir.is_directory()) {
            personas.push_back(dir.path().filename().string());
        }
    }
    return personas;
}

bool Persona_Switcher::activatePersona(const std::string& name) {
    std::string path = personaRoot + "/" + name;
    if (!fs::exists(path)) {
        std::cerr << "[X] Persona not found: " << name << "\n";
        return false;
    }

    std::cout << "[✓] Persona activated: " << name << "\n";
    // Identity switch could be called here, or linked to Ghost_Identity_Manager
    return true;
}
