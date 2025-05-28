#ifndef SECUREDELETION_SIMD_H
#define SECUREDELETION_SIMD_H

#include <string>

class SecureDeletion_SIMD {
public:
    static bool shredFile(const std::string& filename, int passes = 3);
};

#endif
