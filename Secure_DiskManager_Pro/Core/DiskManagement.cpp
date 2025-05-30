#include "DiskManagement.h"
#include <windows.h>
#include <shlwapi.h>
#include <iostream>

uintmax_t DiskManagement::getDiskUsage(const std::string& path) {
    WIN32_FIND_DATA data;
    HANDLE hFind;
    uintmax_t totalSize = 0;

    std::string fullPath = path + "\\*";

    if ((hFind = FindFirstFile(fullPath.c_str(), &data)) != INVALID_HANDLE_VALUE) {
        do {
            if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                LARGE_INTEGER fileSize;
                fileSize.HighPart = data.nFileSizeHigh;
                fileSize.LowPart = data.nFileSizeLow;
                totalSize += fileSize.QuadPart;
            }
        } while (FindNextFile(hFind, &data));
        FindClose(hFind);
    }

    return totalSize;
}
