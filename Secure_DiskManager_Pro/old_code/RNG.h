#ifndef RNG_H
#define RNG_H

#include <vector>
#include <cstdint>

class RNG {
public:
    static bool fillBuffer(uint8_t* buffer, size_t size);
    static std::vector<uint8_t> getRandomBytes(size_t size);
    static uint32_t getRandomInt(uint32_t min, uint32_t max);
};

#endif
