#ifndef FILE_COMPRESSOR_MT_SIMD_H
#define FILE_COMPRESSOR_MT_SIMD_H

#include <string>

class FileCompressorMT {
public:
    static bool compress(const std::string& inputPath, const std::string& outputPath, size_t chunkSize = 1 << 20); // 1MB
    static bool decompress(const std::string& inputPath, const std::string& outputPath);
};

#endif
