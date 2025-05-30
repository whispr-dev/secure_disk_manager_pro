#pragma once
#include <string>

class Ghost_NetUtils {
public:
    static bool changeMacAddress(const std::string& interface, const std::string& newMac);
    static std::string generateRandomMac();
};
