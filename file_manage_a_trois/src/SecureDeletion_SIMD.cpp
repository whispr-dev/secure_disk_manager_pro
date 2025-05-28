#include "SecureDeletion_SIMD.h"
#include "RNG_SIMD.h"
#include <fstream>
#include <filesystem>
#include <immintrin.h> // AVX2
#include <vector>
#include <iostream>

bool SecureDeletion_SIMD::shredFile(const std::string& filename, int passes) {
    using namespace std::filesystem;

    if (!exists(filename)) return false;
    std::error_code ec;
    uintmax_t size = file_size(filename, ec);
    if (ec) return false;

    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) return false;

    std::vector<uint8_t> buffer(size);

    for (int pass = 0; pass < passes; ++pass) {
        if (pass == 0) {
            buffer = RNG_SIMD::getRandomBytes(size);
        } else if (pass == 1) {
            std::fill(buffer.begin(), buffer.end(), 0xFF);
        } else {
            std::fill(buffer.begin(), buffer.end(), 0x00);
        }

        file.seekp(0);
        file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        file.flush();
    }

    file.close();
    return remove(filename.c_str()) == 0;
}
