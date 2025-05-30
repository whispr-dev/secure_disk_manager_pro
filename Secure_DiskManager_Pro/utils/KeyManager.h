#ifndef KEYMANAGER_H
#define KEYMANAGER_H

#include <string>
#include <vector>

class KeyManager {
public:
    static bool generateKeyfile(const std::string& name, size_t size = 4096);
    static bool deleteKeyfile(const std::string& name);
    static std::vector<uint8_t> loadKeyfile(const std::string& name);
    static void listKeyfiles();
    static std::string getKeyDirectory();
};

#endif
