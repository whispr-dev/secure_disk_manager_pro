#ifndef RNG_H
#define RNG_H

#include <vector>
#include <cstdint>

class RNG {
public:
    static void initialize();
    static std::vector<uint8_t> getRandomBytes(size_t size);
    static uint64_t getRandom64();
    static uint64_t getRandomInRange(uint64_t min, uint64_t max);
    static bool writeKeyfile(const std::string& filename, size_t numBytes = 4096);
};

#endif
