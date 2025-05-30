#ifndef SIMD_ENTROPY_H
#define SIMD_ENTROPY_H

#include <string>
#include <vector>
#include <cstdint>

class SIMD_Entropy {
public:
    static double calculateEntropy(const std::string& content);
    static double calculateFileEntropy(const std::string& filepath);
};

#endif
