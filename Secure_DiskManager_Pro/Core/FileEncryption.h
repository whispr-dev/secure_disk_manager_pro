#ifndef FILEENCRYPTION_H
#define FILEENCRYPTION_H

#include <string>
#include <vector>

class FileEncryption {
public:
    bool encryptFile(const std::string& filePath, const std::string& key);
    bool decryptFile(const std::string& filePath, const std::string& key);

    bool encryptFileWithKeyfile(const std::string& filePath, const std::string& keyfilePath);
    bool decryptFileWithKeyfile(const std::string& filePath, const std::string& keyfilePath);

private:
    void initializeSBox(const unsigned char* key, size_t keylen, unsigned char sbox[256]);
    void rc4EncryptDecrypt(unsigned char* data, int length, const unsigned char* key, size_t keylen);
};

#endif