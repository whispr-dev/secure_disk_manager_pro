#ifndef GPG_WRAPPER_H
#define GPG_WRAPPER_H

#include <string>

class GpgWrapper {
public:
    static bool encryptFile(const std::string& filepath, const std::string& recipientKeyID);
    static bool decryptFile(const std::string& filepath, const std::string& outputpath);
    static bool importKey(const std::string& keyfile);
    static bool keyExists(const std::string& keyID);
};

#endif // GPG_WRAPPER_H
