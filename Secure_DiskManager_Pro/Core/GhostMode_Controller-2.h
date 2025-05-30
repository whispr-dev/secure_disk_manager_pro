#ifndef GHOSTMODE_CONTROLLER_H
#define GHOSTMODE_CONTROLLER_H

#include <string>
#include <vector>

class GhostMode_Controller {
public:
    GhostMode_Controller();

    // Initialization and shutdown
    bool Initialize();
    void Shutdown();

    // Persona and session management
    bool LoadPersona(const std::string& personaName);
    bool SavePersonaState();
    std::vector<std::string> ListAvailablePersonas();

    // MAC spoofing + GPG identity swap
    bool ApplyMACSpoofing();
    bool ApplyGPGIdentity();

    // Stealth features
    void EnableStealthMode();
    void DisableStealthMode();

    // System state info
    std::string GetCurrentPersona() const;
    bool IsInitialized() const;

private:
    std::string activePersona;
    bool initialized;

    bool LoadMACConfig(const std::string& personaPath);
    bool LoadGPGKey(const std::string& personaPath);
};

#endif // GHOSTMODE_CONTROLLER_H
