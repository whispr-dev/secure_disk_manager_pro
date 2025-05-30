#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <string>

class ServiceManager {
public:
    static void listServices();
    static bool startService(const std::string& serviceName);
    static bool stopService(const std::string& serviceName);
};

#endif
