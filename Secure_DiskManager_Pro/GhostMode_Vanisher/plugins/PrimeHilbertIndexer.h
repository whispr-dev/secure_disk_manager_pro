#ifndef PRIME_HILBERT_INDEXER_H
#define PRIME_HILBERT_INDEXER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

struct HilbertCoord {
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

class PrimeHilbertIndexer {
public:
    static std::vector<std::string> tokenize(const std::string& content);
    static std::vector<uint64_t> primeIndices(const std::vector<std::string>& tokens);
    static HilbertCoord toHilbert3D(uint64_t index);

    static HilbertCoord hashFileToHilbert(const std::string& filepath);
};

#endif
