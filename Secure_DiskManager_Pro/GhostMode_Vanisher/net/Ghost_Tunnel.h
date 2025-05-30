#ifndef GHOST_TUNNEL_H
#define GHOST_TUNNEL_H

#include <string>
#include <thread>
#include <atomic>

class GhostTunnel {
public:
    GhostTunnel();
    ~GhostTunnel();

    bool initialize(const std::string& proxyAddress, int proxyPort, const std::string& encryptionKey);
    bool startTunnel();
    void stopTunnel();
    bool isRunning() const;

private:
    void tunnelLoop();
    bool connectToProxy();
    void encryptAndRelayTraffic();

    std::string proxyAddress_;
    int proxyPort_;
    std::string encryptionKey_;
    std::thread tunnelThread_;
    std::atomic<bool> running_;
    int proxySocket_;

    bool setupSocket();
    void closeSocket();
};

#endif // GHOST_TUNNEL_H
