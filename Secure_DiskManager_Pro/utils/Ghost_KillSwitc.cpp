#include <iostream>
#include <filesystem>
#include <cstdlib>

void GhostKillSwitch::triggerKillSwitch(bool fullWipe) {
    terminateTunnels();
    wipeIdentities();
    wipeConfigs();
    if (fullWipe) selfDelete();
}

void GhostKillSwitch::terminateTunnels() {
    std::system("pkill -f ghost_tunnel || taskkill /IM ghost_tunnel.exe /F");
}

void GhostKillSwitch::wipeIdentities() {
    std::filesystem::remove_all("~/.ghostmode/identities");
}

void GhostKillSwitch::wipeConfigs() {
    std::filesystem::remove_all("~/.ghostmode/config");
}

void GhostKillSwitch::selfDelete() {
#ifdef _WIN32
    std::system("timeout 3 && del %~f0");
#else
    std::system("shred -u $(readlink -f $0)");
#endif
}