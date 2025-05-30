#ifndef GHOSTMODE_CONTROLLER_H
#define GHOSTMODE_CONTROLLER_H

#include "Ghost_NetUtils.h"
#include "Ghost_AutoWipe.h"
#include "Ghost_FS_Monitor.h"
#include "Ghost_UI_CLI.h"
#include "Ghost_Identity_Manager.h"
#include "Persona_Switcher.h"
#include "Ghost_Tunnel.h"
#include "Ghost_KillSwitch.h"
#include "GhostDaemon.h"
#include "GhostWatchdog.h"
#include "GhostStealthNet.h"

class GhostModeController {
public:
    GhostModeController();

    void initializeStealthOps();
    void configureWatchdog();

    void startAll();
    void stopAll();
    void checkStatus();

    void triggerKillSwitch();      // <- Kill switch activation
    void installDaemon();          // <- Daemon install
    void uninstallDaemon();        // <- Daemon uninstall
    void startDaemon();            // <- Daemon start
    void stopDaemon();             // <- Daemon stop
    void Init();
    void Run();
    void StartTunneling(const std::string& endpoint);
    void StopTunneling();
    void RotateIdentity();

private:
    GhostNetUtils netUtils;
    GhostAutoWipe autoWipe;
    GhostFSMonitor fsMonitor;
    GhostUICLI cliUI;
    GhostIdentityManager idManager;
    PersonaSwitcher personaSwitcher;
    GhostTunnel tunnel;
    GhostKillSwitch killSwitch;
    GhostDaemon daemon;
    GhostWatchdog watchdog;
    GhostStealthNet stealthNet;
};

#endif // GHOSTMODE_CONTROLLER_H
