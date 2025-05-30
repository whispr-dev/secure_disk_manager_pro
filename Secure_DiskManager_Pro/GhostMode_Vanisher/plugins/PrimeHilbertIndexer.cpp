#include "PrimeHilbertIndexer.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <unordered_set>

std::vector<std::string> PrimeHilbertIndexer::tokenize(const std::string& content) {
    std::istringstream iss(content);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool isPrime(uint64_t n) {
    if (n < 2) return false;
    for (uint64_t i = 2; i * i <= n; ++i) {
        if (n % i == 0) return false;
    }
    return true;
}

std::vector<uint64_t> PrimeHilbertIndexer::primeIndices(const std::vector<std::string>& tokens) {
    std::vector<uint64_t> primes;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (isPrime(i)) {
            primes.push_back(i);
        }
    }
    return primes;
}

// Simplified Hilbert 3D encoding (non-optimal, mock-up for conceptual locality mapping)
HilbertCoord PrimeHilbertIndexer::toHilbert3D(uint64_t index) {
    HilbertCoord c;
    c.x = (index >> 0) & 0xFF;
    c.y = (index >> 8) & 0xFF;
    c.z = (index >> 16) & 0xFF;
    return c;
}

HilbertCoord PrimeHilbertIndexer::hashFileToHilbert(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file) return {0, 0, 0};

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    auto tokens = tokenize(content);
    auto primes = primeIndices(tokens);

    uint64_t sum = 0;
    for (auto i : primes) {
        sum += i * i;
    }

    return toHilbert3D(sum);
}
