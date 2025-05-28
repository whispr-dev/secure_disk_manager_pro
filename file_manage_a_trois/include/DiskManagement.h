#ifndef DISKMANAGEMENT_H
#define DISKMANAGEMENT_H

#include <string>
#include <cstdint>

class DiskManagement {
public:
    static uintmax_t getDiskUsage(const std::string& path);
};

#endif
