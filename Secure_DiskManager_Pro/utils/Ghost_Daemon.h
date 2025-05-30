#define GHOST_DAEMON_H

class GhostDaemon {
public:
    static void start();
    static void stop();
    static void install();
    static void uninstall();
};

#endif // GHOST_DAEMON_H