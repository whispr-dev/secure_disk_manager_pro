#include <iostream>
#include <cstdlib>

void GhostDaemon::start() {
    std::cout << "[Daemon] Starting GhostMode services..." << std::endl;
    std::system("nohup ghost_identity_rotator &");
    std::system("nohup ghost_tunnel_watchdog &");
    std::system("nohup ghost_autowipe_countdown &");
}

void GhostDaemon::stop() {
    std::cout << "[Daemon] Stopping GhostMode services..." << std::endl;
    std::system("pkill -f ghost_identity_rotator");
    std::system("pkill -f ghost_tunnel_watchdog");
    std::system("pkill -f ghost_autowipe_countdown");
}

void GhostDaemon::install() {
    std::cout << "[Daemon] Installing GhostMode daemon..." << std::endl;
    // Placeholder: add systemd or Windows Service registration
}

void GhostDaemon::uninstall() {
    std::cout << "[Daemon] Uninstalling GhostMode daemon..." << std::endl;
    // Placeholder: remove systemd or Windows Service registration
}