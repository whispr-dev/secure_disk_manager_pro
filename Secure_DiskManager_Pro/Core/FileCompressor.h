#ifndef FILE_COMPRESSOR_H
#define FILE_COMPRESSOR_H

#include <string>

class FileCompressor {
public:
    static bool compress(const std::string& inputPath, const std::string& outputPath);
    static bool decompress(const std::string& inputPath, const std::string& outputPath);
};

#endif
