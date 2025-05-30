#ifndef GHOST_STEALTH_NET_H
#define GHOST_STEALTH_NET_H

#include <string>

class GhostStealthNet {
public:
    GhostStealthNet();
    void enableDnsCloak();
    void setupProxyTunnel(const std::string& proxyAddress);
    void heartbeat();

private:
    std::string currentProxy;
};

#endif // GHOST_STEALTH_NET_H
