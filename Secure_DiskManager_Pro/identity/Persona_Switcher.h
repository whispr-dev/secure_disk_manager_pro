#ifndef PERSONA_SWITCHER_H
#define PERSONA_SWITCHER_H

#include <string>
#include <vector>

class Persona_Switcher {
public:
    Persona_Switcher();

    void setPersonaRoot(const std::string& rootPath);
    std::vector<std::string> listPersonas();
    bool activatePersona(const std::string& name);

private:
    std::string personaRoot;
};

#endif // PERSONA_SWITCHER_H
