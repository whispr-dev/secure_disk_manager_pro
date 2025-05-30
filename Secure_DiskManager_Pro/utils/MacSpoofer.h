#ifndef MAC_SPOOFER_H
#define MAC_SPOOFER_H

#include <string>

class MacSpoofer {
public:
    static bool spoofMac(const std::string& interfaceName, const std::string& newMac);
    static std::string generateRandomMac();
    static std::string getCurrentMac(const std::string& interfaceName);
};

#endif // MAC_SPOOFER_H
