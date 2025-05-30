#include "mac_spoofer.h"
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <random>
#include <array>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

std::string MacSpoofer::generateRandomMac() {
    std::array<unsigned char, 6> mac;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    mac[0] = dis(gen) & 0xFE;  // Unicast, locally administered
    for (int i = 1; i < 6; ++i)
        mac[i] = dis(gen);

    std::ostringstream oss;
    for (size_t i = 0; i < mac.size(); ++i) {
        if (i > 0) oss << ":";
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(mac[i]);
    }

    return oss.str();
}

bool MacSpoofer::spoofMac(const std::string& interfaceName, const std::string& newMac) {
#ifdef _WIN32
    std::cerr << "[!] MAC spoofing not implemented for Windows in this version.\n";
    return false;
#else
    std::string cmdDown = "sudo ip link set " + interfaceName + " down";
    std::string cmdSpoof = "sudo ip link set " + interfaceName + " address " + newMac;
    std::string cmdUp = "sudo ip link set " + interfaceName + " up";

    int result = std::system(cmdDown.c_str());
    result |= std::system(cmdSpoof.c_str());
    result |= std::system(cmdUp.c_str());

    return result == 0;
#endif
}

std::string MacSpoofer::getCurrentMac(const std::string& interfaceName) {
#ifdef _WIN32
    return "[Windows-MAC-unavailable]";
#else
    std::string cmd = "cat /sys/class/net/" + interfaceName + "/address";
    char buffer[32];
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "error";
    fgets(buffer, sizeof(buffer), pipe);
    pclose(pipe);
    return std::string(buffer);
#endif
}
