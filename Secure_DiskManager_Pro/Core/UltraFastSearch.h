#ifndef ULTRAFASTSEARCH_H
#define ULTRAFASTSEARCH_H

#include <string>
#include <vector>

struct FileEntry {
    std::string fullPath;
    uintmax_t size;
    std::string modifiedTime;
};

class UltraFastSearch {
public:
    static std::vector<FileEntry> search(const std::string& pattern, const std::string& rootPath = "C:\\");
    static void printResults(const std::vector<FileEntry>& results);
    static bool exportToCSV(const std::vector<FileEntry>& results, const std::string& outFile);
};

#endif
