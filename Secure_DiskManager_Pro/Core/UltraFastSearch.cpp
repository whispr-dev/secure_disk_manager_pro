#include "UltraFastSearch.h"
#include <windows.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <chrono>

namespace fs = std::filesystem;

static std::string format_time(const std::filesystem::file_time_type& ftime) {
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now()
        + std::chrono::system_clock::now());

    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
    char buf[64];
    ctime_s(buf, sizeof(buf), &cftime);
    return std::string(buf);
}

std::vector<FileEntry> UltraFastSearch::search(const std::string& pattern, const std::string& rootPath) {
    std::vector<FileEntry> results;

    for (auto& p : fs::recursive_directory_iterator(rootPath, fs::directory_options::skip_permission_denied)) {
        try {
            if (!p.is_regular_file()) continue;
            std::string path = p.path().string();
            if (path.find(pattern) != std::string::npos) {
                FileEntry entry;
                entry.fullPath = path;
                entry.size = fs::file_size(p);
                entry.modifiedTime = format_time(fs::last_write_time(p));
                results.push_back(entry);
            }
        } catch (...) {
            continue;
        }
    }

    return results;
}

void UltraFastSearch::printResults(const std::vector<FileEntry>& results) {
    for (const auto& entry : results) {
        std::cout << entry.fullPath << " | Size: " << entry.size << " bytes | Modified: " << entry.modifiedTime;
    }
}

bool UltraFastSearch::exportToCSV(const std::vector<FileEntry>& results, const std::string& outFile) {
    std::ofstream out(outFile);
    if (!out) return false;

    out << "Path,Size,Modified\n";
    for (const auto& entry : results) {
        out << "\"" << entry.fullPath << "\"," << entry.size << ",\"" << entry.modifiedTime << "\"\n";
    }

    return true;
}
