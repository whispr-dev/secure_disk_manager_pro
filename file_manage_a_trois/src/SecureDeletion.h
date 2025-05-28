#ifndef SECUREDELETION_H
#define SECUREDELETION_H

#include <string>

class SecureDeletion {
public:
    static bool shredFile(const std::string& filename, int passes = 3);
};

#endif
