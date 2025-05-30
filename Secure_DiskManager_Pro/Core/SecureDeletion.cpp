// Secure Deletion
#include "SecureDeletion.h"
#include <fstream>
#include <random>
#include <filesystem>

bool SecureDeletion::shredFile(const std::string& filepath, int passes) {
    if (!std::filesystem::exists(filepath)) return false;

    std::error_code ec;
    std::uintmax_t size = std::filesystem::file_size(filepath, ec);
    if (ec) return false;

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, 255);

    for (int pass = 0; pass < passes; ++pass) {
        std::ofstream file(filepath, std::ios::binary | std::ios::in | std::ios::out);
        if (!file) return false;

        for (std::uintmax_t i = 0; i < size; ++i) {
            char junk = static_cast<char>(dist(rng));
            file.seekp(i);
            file.write(&junk, 1);
        }

        file.close();
    }

    return std::filesystem::remove(filepath);
}
