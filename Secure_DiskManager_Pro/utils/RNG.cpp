// RNG module
#include "RNG.h"
#include <windows.h>
#include <wincrypt.h>
#include <stdexcept>

bool RNG::fillBuffer(uint8_t* buffer, size_t size) {
    HCRYPTPROV hProvider = 0;
    if (!CryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        return false;
    }

    bool success = CryptGenRandom(hProvider, static_cast<DWORD>(size), buffer);
    CryptReleaseContext(hProvider, 0);
    return success;
}

std::vector<uint8_t> RNG::getRandomBytes(size_t size) {
    std::vector<uint8_t> data(size);
    if (!fillBuffer(data.data(), size)) {
        throw std::runtime_error("Failed to generate random bytes");
    }
    return data;
}

uint32_t RNG::getRandomInt(uint32_t min, uint32_t max) {
    if (min >= max) return min;

    uint32_t range = max - min + 1;
    uint32_t result;
    do {
        uint32_t rnd = 0;
        if (!fillBuffer(reinterpret_cast<uint8_t*>(&rnd), sizeof(rnd))) {
            throw std::runtime_error("Failed to generate random integer");
        }
        result = rnd % range;
    } while (result >= range);
    return min + result;
}
