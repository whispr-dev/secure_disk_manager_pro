// File: src/GhostMode_Controller.cpp
#include "GhostMode_Controller.h"
#include "GhostWatchdog.h"
#include "GhostStealthNet.h"

#include <iostream>

GhostMode_Controller::GhostMode_Controller() {}

void GhostMode_Controller::Init() {
    cliUI.Init();
    netUtils.ConfigureFirewallRules();
    fsMonitor.MonitorStart();
    idManager.LoadIdentities();
    personaSwitcher.Preload();
    tunnel.Configure();  // Setup the tunnel
    std::cout << "[GhostMode] Initialization complete.\n";

void GhostModeController::initializeStealthOps() {
    stealthNet.enableDnsCloak();
    stealthNet.setupProxyTunnel("127.0.0.1:9050"); // example: TOR or custom socks
    stealthNet.heartbeat();
}

void GhostModeController::configureWatchdog() {
    watchdog.addWatchedProcess("GhostCLI_Main.exe");
    watchdog.addWatchedProcess("GhostNetDaemon.exe");
    watchdog.start();
}

void GhostModeController::initialize() {
    std::cout << "[*] GhostMode initializing core subsystems...\n";
    initializeStealthOps();
    configureWatchdog();
    // other systems like identity switcher, MAC spoofer, GPG loader...
}

}

GhostModeController::GhostModeController()
    : killSwitch(), daemon() {}

void GhostModeController::startAll() {
    std::cout << "[+] Starting all GhostMode subsystems...\n";
    daemon.start();
}

void GhostModeController::stopAll() {
    std::cout << "[-] Stopping all GhostMode subsystems...\n";
    daemon.stop();

void GhostModeController::shutdown() {
    std::cout << "[*] GhostMode shutting down...\n";
    watchdog.stop();
    // any other cleanup
}

}

void GhostModeController::checkStatus() {
    std::cout << "[=] GhostMode Status:\n";
    daemon.status();
}

void GhostModeController::triggerKillSwitch() {
    std::cout << "[!] Triggering emergency kill switch!\n";
    killSwitch.activate();
}

void GhostModeController::installDaemon() {
    std::cout << "[+] Installing GhostMode daemon...\n";
    daemon.install();
}

void GhostModeController::uninstallDaemon() {
    std::cout << "[-] Uninstalling GhostMode daemon...\n";
    daemon.uninstall();
}

void GhostModeController::startDaemon() {
    std::cout << "[>] Starting GhostMode daemon...\n";
    daemon.start();
}

void GhostModeController::stopDaemon() {
    std::cout << "[<] Stopping GhostMode daemon...\n";
    daemon.stop();
}

void GhostMode_Controller::Run() {
    std::cout << "[GhostMode] Running in secure mode...\n";
    cliUI.MainLoop(*this);
}

void GhostMode_Controller::StartTunneling(const std::string& endpoint) {
    std::cout << "[GhostMode] Starting tunnel to " << endpoint << "...\n";
    if (!tunnel.Establish(endpoint)) {
        std::cerr << "[GhostMode] Tunnel establishment failed!\n";
    }
}

void GhostMode_Controller::StopTunneling() {
    std::cout << "[GhostMode] Stopping tunnel...\n";
    tunnel.Terminate();
}

void GhostMode_Controller::RotateIdentity() {
    std::cout << "[GhostMode] Rotating identity...\n";
    personaSwitcher.SwitchPersona();
    idManager.ReloadCurrent();
    tunnel.Rekey();  // Re-establish tunnel with new identity
}
