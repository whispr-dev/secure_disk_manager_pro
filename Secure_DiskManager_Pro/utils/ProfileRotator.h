#ifndef PROFILE_ROTATOR_H
#define PROFILE_ROTATOR_H

#include <string>
#include <vector>
#include <map>

struct Persona {
    std::string name;
    std::string macAddress;
    std::string gpgKeyID;
    std::string gpgKeyFile;
};

class ProfileRotator {
public:
    ProfileRotator(const std::string& iface);
    bool loadPersonas(const std::string& configFile);
    bool rotateToNext();
    const Persona& getCurrentPersona() const;

private:
    std::string interface;
    std::vector<Persona> personas;
    size_t currentIndex = 0;

    bool applyPersona(const Persona& p);
};

#endif // PROFILE_ROTATOR_H
