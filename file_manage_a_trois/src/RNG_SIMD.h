#ifndef RNG_SIMD_H
#define RNG_SIMD_H

#include <vector>
#include <string>
#include <random>
#include <cstdint>

class RNG_SIMD {
public:
    static void initialize();
    static uint64_t getRandom64();
    static std::vector<uint8_t> getRandomBytes(size_t size);
    static uint64_t getRandomInRange(uint64_t min, uint64_t max);
    static bool writeKeyfile(const std::string& filename, size_t numBytes = 4096);
};

#endif
