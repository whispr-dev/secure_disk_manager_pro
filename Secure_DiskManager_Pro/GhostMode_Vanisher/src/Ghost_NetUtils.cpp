#include "Ghost_NetUtils.h"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

bool Ghost_NetUtils::changeMacAddress(const std::string& interface, const std::string& newMac) {
    std::ostringstream cmd;
#ifdef _WIN32
    std::cerr << "[Ghost_NetUtils] Windows MAC spoofing not implemented yet.\n";
    return false;
#else
    cmd << "sudo ifconfig " << interface << " down && "
        << "sudo ifconfig " << interface << " hw ether " << newMac << " && "
        << "sudo ifconfig " << interface << " up";
#endif

    int result = std::system(cmd.str().c_str());
    return result == 0;
}

std::string Ghost_NetUtils::generateRandomMac() {
    std::ostringstream mac;
    mac << "02"; // Locally administered, unicast
    for (int i = 0; i < 5; ++i) {
        mac << ":" << std::hex << (rand() % 256);
    }
    return mac.str();
}
