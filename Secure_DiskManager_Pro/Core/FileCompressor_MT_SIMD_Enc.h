#ifndef FILE_COMPRESSOR_MT_SIMD_ENC_H
#define FILE_COMPRESSOR_MT_SIMD_ENC_H

#include <string>

class FileCompressorMTEnc {
public:
    static bool compress(const std::string& inputPath, const std::string& outputPath, const std::string& keyfile = "", const std::string& mode = "speed");
    static bool decompress(const std::string& inputPath, const std::string& outputPath, const std::string& keyfile = "");
};

#endif
