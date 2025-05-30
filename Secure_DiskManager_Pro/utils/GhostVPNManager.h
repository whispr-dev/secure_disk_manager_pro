#ifndef GHOST_VPN_MANAGER_H
#define GHOST_VPN_MANAGER_H

#include <string>

namespace GhostVPNManager {

    // Starts the VPN with a given profile name (stubbed)
    bool start_vpn_connection(const std::string& profileName);

    // Checks if a VPN connection is active (stubbed)
    bool is_vpn_active();

} // namespace GhostVPNManager

#endif // GHOST_VPN_MANAGER_H
