#include "RNG_SIMD.h"
#include <windows.h>
#include <fstream>
#include <random>
#include <immintrin.h>

static std::mt19937_64 mt_engine;

void RNG_SIMD::initialize() {
    uint64_t seed = 0;
    HCRYPTPROV hProvider;
    if (CryptAcquireContext(&hProvider, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CryptGenRandom(hProvider, sizeof(seed), reinterpret_cast<BYTE*>(&seed));
        CryptReleaseContext(hProvider, 0);
    } else {
        seed = static_cast<uint64_t>(GetTickCount64());
    }
    mt_engine.seed(seed);
}

uint64_t RNG_SIMD::getRandom64() {
    return mt_engine();
}

std::vector<uint8_t> RNG_SIMD::getRandomBytes(size_t size) {
    std::vector<uint8_t> buffer(size);
    size_t i = 0;
    while (i + 32 <= size) {
        __m256i rnd = _mm256_set_epi64x(mt_engine(), mt_engine(), mt_engine(), mt_engine());
        _mm256_storeu_si256((__m256i*)(&buffer[i]), rnd);
        i += 32;
    }
    while (i < size) {
        buffer[i++] = static_cast<uint8_t>(mt_engine() & 0xFF);
    }
    return buffer;
}

uint64_t RNG_SIMD::getRandomInRange(uint64_t min, uint64_t max) {
    if (min >= max) return min;
    return min + (getRandom64() % (max - min + 1));
}

bool RNG_SIMD::writeKeyfile(const std::string& filename, size_t numBytes) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) return false;
    auto key = getRandomBytes(numBytes);
    out.write(reinterpret_cast<const char*>(key.data()), key.size());
    return true;
}
