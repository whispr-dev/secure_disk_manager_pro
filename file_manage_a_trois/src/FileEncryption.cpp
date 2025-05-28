// file_encryption.cpp
#include <fstream>
#include <vector>
#include <iostream>

void FileEncryption::initializeSBox(const unsigned char* key, size_t keylen, unsigned char sbox[256]) {
    for (int i = 0; i < 256; i++) {
        sbox[i] = static_cast<unsigned char>(i);
    }
    int j = 0;
    for (int i = 0; i < 256; i++) {
        j = (j + sbox[i] + key[i % keylen]) % 256;
        std::swap(sbox[i], sbox[j]);
    }
}

void FileEncryption::rc4EncryptDecrypt(unsigned char* data, int length, const unsigned char* key, size_t keylen) {
    unsigned char sbox[256];
    initializeSBox(key, keylen, sbox);

    int i = 0, j = 0;
    for (int n = 0; n < length; n++) {
        i = (i + 1) % 256;
        j = (j + sbox[i]) % 256;
        std::swap(sbox[i], sbox[j]);
        int k = sbox[(sbox[i] + sbox[j]) % 256];
        data[n] ^= k;
    }
}

bool FileEncryption::encryptFile(const std::string& filePath, const std::string& key) {
    std::ifstream in(filePath, std::ios::binary);
    if (!in) return false;

    std::vector<unsigned char> data((std::istreambuf_iterator<char>(in)), {});
    in.close();

    rc4EncryptDecrypt(data.data(), data.size(),
                      reinterpret_cast<const unsigned char*>(key.data()), key.size());

    std::ofstream out(filePath + ".enc", std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<char*>(data.data()), data.size());
    out.close();
    return true;
}

bool FileEncryption::decryptFile(const std::string& filePath, const std::string& key) {
    std::ifstream in(filePath, std::ios::binary);
    if (!in) return false;

    std::vector<unsigned char> data((std::istreambuf_iterator<char>(in)), {});
    in.close();

    rc4EncryptDecrypt(data.data(), data.size(),
                      reinterpret_cast<const unsigned char*>(key.data()), key.size());

    std::ofstream out(filePath + ".dec", std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<char*>(data.data()), data.size());
    out.close();
    return true;
}

bool FileEncryption::encryptFileWithKeyfile(const std::string& filePath, const std::string& keyfilePath) {
    std::ifstream keyfile(keyfilePath, std::ios::binary);
    if (!keyfile) return false;
    std::vector<unsigned char> key((std::istreambuf_iterator<char>(keyfile)), {});
    keyfile.close();

    if (key.empty()) return false;

    std::ifstream in(filePath, std::ios::binary);
    if (!in) return false;

    std::vector<unsigned char> data((std::istreambuf_iterator<char>(in)), {});
    in.close();

    rc4EncryptDecrypt(data.data(), data.size(), key.data(), key.size());

    std::ofstream out(filePath + ".enc", std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<char*>(data.data()), data.size());
    out.close();
    return true;
}

bool FileEncryption::decryptFileWithKeyfile(const std::string& filePath, const std::string& keyfilePath) {
    std::ifstream keyfile(keyfilePath, std::ios::binary);
    if (!keyfile) return false;
    std::vector<unsigned char> key((std::istreambuf_iterator<char>(keyfile)), {});
    keyfile.close();

    if (key.empty()) return false;

    std::ifstream in(filePath, std::ios::binary);
    if (!in) return false;

    std::vector<unsigned char> data((std::istreambuf_iterator<char>(in)), {});
    in.close();

    rc4EncryptDecrypt(data.data(), data.size(), key.data(), key.size());

    std::ofstream out(filePath + ".dec", std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<char*>(data.data()), data.size());
    out.close();
    return true;
}
