#include "FileCompressor.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>
#include <zlib.h> // Using zlib's deflate/inflate — fast and portable

bool FileCompressor::compress(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream in(inputPath, std::ios::binary);
    if (!in) return false;

    std::vector<char> inputBuffer((std::istreambuf_iterator<char>(in)), {});
    uLong srcSize = inputBuffer.size();
    uLong destSize = compressBound(srcSize);
    std::vector<Bytef> outputBuffer(destSize);

    int res = compress2(outputBuffer.data(), &destSize, (Bytef*)inputBuffer.data(), srcSize, Z_BEST_SPEED);
    if (res != Z_OK) return false;

    std::ofstream out(outputPath, std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<char*>(&srcSize), sizeof(uLong)); // Write original size
    out.write(reinterpret_cast<char*>(outputBuffer.data()), destSize);
    return true;
}

bool FileCompressor::decompress(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream in(inputPath, std::ios::binary);
    if (!in) return false;

    uLong srcSize;
    in.read(reinterpret_cast<char*>(&srcSize), sizeof(uLong));

    std::vector<char> inputBuffer((std::istreambuf_iterator<char>(in)), {});
    std::vector<Bytef> outputBuffer(srcSize);

    uLong destSize = srcSize;
    int res = uncompress(outputBuffer.data(), &destSize, (Bytef*)inputBuffer.data(), inputBuffer.size());
    if (res != Z_OK) return false;

    std::ofstream out(outputPath, std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<char*>(outputBuffer.data()), destSize);
    return true;
}
