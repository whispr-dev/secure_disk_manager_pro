#define GHOST_KILLSWITCH_H

class GhostKillSwitch {
public:
    static void triggerKillSwitch(bool fullWipe = false);
    static void wipeIdentities();
    static void terminateTunnels();
    static void wipeConfigs();
    static void selfDelete();
};

#endif // GHOST_KILLSWITCH_H