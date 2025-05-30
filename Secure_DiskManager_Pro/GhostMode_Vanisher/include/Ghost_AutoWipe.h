#pragma once
#include <string>

class Ghost_AutoWipe {
public:
    Ghost_AutoWipe(const std::string& path, int delaySeconds);

    void arm();
    void abort();

private:
    std::string watchPath;
    int delay;
    bool armed;
};
