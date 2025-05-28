#include "KeyManager.h"
#include "RNG_SIMD.h"
#include "SecureDeletion_SIMD.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

std::string KeyManager::getKeyDirectory() {
    std::string dir = "./keys/";
    fs::create_directories(dir);
    return dir;
}

bool KeyManager::generateKeyfile(const std::string& name, size_t size) {
    RNG_SIMD::initialize();
    std::string path = getKeyDirectory() + name;
    return RNG_SIMD::writeKeyfile(path, size);
}

bool KeyManager::deleteKeyfile(const std::string& name) {
    std::string path = getKeyDirectory() + name;
    return SecureDeletion_SIMD::shredFile(path);
}

std::vector<uint8_t> KeyManager::loadKeyfile(const std::string& name) {
    std::string path = getKeyDirectory() + name;
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)),
                              std::istreambuf_iterator<char>());
    return data;
}

void KeyManager::listKeyfiles() {
    std::string dir = getKeyDirectory();
    for (const auto& f : fs::directory_iterator(dir)) {
        std::cout << f.path().filename().string() << "\n";
    }
}
