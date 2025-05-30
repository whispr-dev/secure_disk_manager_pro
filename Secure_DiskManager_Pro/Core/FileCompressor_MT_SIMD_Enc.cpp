#include "FileCompressor_MT_SIMD_Enc.h"
#include "../Crypto.h"  // Assumes RC4-style cipher exists here
#include <fstream>
#include <vector>
#include <thread>
#include <future>
#include <iostream>
#include <zlib.h>
#include <immintrin.h>

static bool isLowEntropySIMD(const uint8_t* data, size_t len) {
    if (!__builtin_cpu_supports("avx2")) return false;
    __m256i prev = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data));
    for (size_t i = 32; i + 32 <= len; i += 32) {
        __m256i curr = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data + i));
        __m256i diff = _mm256_xor_si256(prev, curr);
        if (_mm256_movemask_epi8(diff)) return false;
        prev = curr;
    }
    return true;
}

static std::vector<uint8_t> loadKey(const std::string& keyfile) {
    std::ifstream kf(keyfile, std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(kf)), {});
}

static void xorCrypt(std::vector<uint8_t>& data, const std::vector<uint8_t>& key) {
    for (size_t i = 0; i < data.size(); ++i)
        data[i] ^= key[i % key.size()];
}

bool FileCompressorMTEnc::compress(const std::string& inputPath, const std::string& outputPath, const std::string& keyfile, const std::string& mode) {
    std::ifstream in(inputPath, std::ios::binary);
    if (!in) return false;

    std::vector<uint8_t> inputData((std::istreambuf_iterator<char>(in)), {});
    size_t originalSize = inputData.size();
    size_t chunkSize = 1 << 20;
    size_t numChunks = (originalSize + chunkSize - 1) / chunkSize;
    int level = (mode == "ratio") ? Z_BEST_COMPRESSION : Z_BEST_SPEED;

    std::vector<uint8_t> key;
    if (!keyfile.empty()) key = loadKey(keyfile);

    std::vector<std::future<std::pair<std::vector<uint8_t>, uint32_t>>> tasks;

    for (size_t i = 0; i < numChunks; ++i) {
        size_t start = i * chunkSize;
        size_t size = std::min(chunkSize, originalSize - start);
        const uint8_t* chunkData = inputData.data() + start;

        tasks.push_back(std::async(std::launch::async, [chunkData, size, level, &key]() {
            std::vector<uint8_t> outBuf(compressBound(size));
            uLongf compSize = outBuf.size();

            if (isLowEntropySIMD(chunkData, size)) {
                outBuf.assign(chunkData, chunkData + size);
                if (!key.empty()) xorCrypt(outBuf, key);
                return std::make_pair(outBuf, static_cast<uint32_t>(0));
            }

            int res = compress2(outBuf.data(), &compSize, chunkData, size, level);
            if (res != Z_OK) return std::make_pair(std::vector<uint8_t>{}, static_cast<uint32_t>(-1));
            outBuf.resize(compSize);
            if (!key.empty()) xorCrypt(outBuf, key);
            return std::make_pair(outBuf, static_cast<uint32_t>(compSize));
        }));
    }

    std::ofstream out(outputPath, std::ios::binary);
    if (!out) return false;

    out.write(reinterpret_cast<const char*>(&numChunks), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(&chunkSize), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(&originalSize), sizeof(uint64_t));

    std::vector<std::pair<std::vector<uint8_t>, uint32_t>> results;
    for (auto& task : tasks) results.push_back(task.get());

    for (const auto& [_, size] : results)
        out.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
    for (const auto& [data, _] : results)
        out.write(reinterpret_cast<const char*>(data.data()), data.size());

    return true;
}

bool FileCompressorMTEnc::decompress(const std::string& inputPath, const std::string& outputPath, const std::string& keyfile) {
    std::ifstream in(inputPath, std::ios::binary);
    if (!in) return false;

    uint32_t numChunks, chunkSize;
    uint64_t originalSize;
    in.read(reinterpret_cast<char*>(&numChunks), sizeof(uint32_t));
    in.read(reinterpret_cast<char*>(&chunkSize), sizeof(uint32_t));
    in.read(reinterpret_cast<char*>(&originalSize), sizeof(uint64_t));

    std::vector<uint32_t> compSizes(numChunks);
    for (uint32_t& s : compSizes)
        in.read(reinterpret_cast<char*>(&s), sizeof(uint32_t));

    std::vector<uint8_t> key;
    if (!keyfile.empty()) key = loadKey(keyfile);

    std::vector<uint8_t> output(originalSize);
    size_t writePos = 0;

    for (uint32_t i = 0; i < numChunks; ++i) {
        size_t outSize = std::min<size_t>(chunkSize, originalSize - writePos);

        if (compSizes[i] == 0) {
            in.read(reinterpret_cast<char*>(output.data() + writePos), outSize);
            if (!key.empty()) xorCrypt(std::vector<uint8_t>(output.begin() + writePos, output.begin() + writePos + outSize), key);
        } else {
            std::vector<uint8_t> compBuf(compSizes[i]);
            in.read(reinterpret_cast<char*>(compBuf.data()), compBuf.size());
            if (!key.empty()) xorCrypt(compBuf, key);
            uLongf dstSize = outSize;
            uncompress(output.data() + writePos, &dstSize, compBuf.data(), compBuf.size());
        }

        writePos += outSize;
    }

    std::ofstream out(outputPath, std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<const char*>(output.data()), output.size());
    return true;
}
