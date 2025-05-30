#ifndef COMMAND_CONTEXT_H
#define COMMAND_CONTEXT_H

#include <string>
#include <vector>

struct CommandContext {
    std::vector<std::string> args;
    std::string searchQuery;
    std::string inputFile;
    std::string outputFile;
    std::string keyFile;
    std::string serverUrl;
    std::string mode;
    int retries = 3;
    bool encrypt = false;
    bool decrypt = false;
    bool compress = false;
    bool decompress = false;
    bool upload = false;
    bool download = false;
    bool search = false;
    bool shred = false;
    bool selfdelete = false;
};

#endif // COMMAND_CONTEXT_H
