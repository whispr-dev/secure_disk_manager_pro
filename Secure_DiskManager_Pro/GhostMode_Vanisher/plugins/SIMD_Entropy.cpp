#include "SIMD_Entropy.h"
#include <fstream>
#include <cmath>
#include <array>
#include <immintrin.h>

double SIMD_Entropy::calculateEntropy(const std::string& content) {
    std::array<uint32_t, 256> freq = {0};
    size_t total = content.size();

    for (unsigned char c : content) {
        freq[c]++;
    }

    double entropy = 0.0;
    for (auto f : freq) {
        if (f > 0) {
            double p = static_cast<double>(f) / total;
            entropy -= p * std::log2(p);
        }
    }

    return entropy;
}

double SIMD_Entropy::calculateFileEntropy(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) return 0.0;

    std::string data((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());

    return calculateEntropy(data);
}
