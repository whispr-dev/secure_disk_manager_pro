#ifndef FILE_TRANSFER_MANAGER_H
#define FILE_TRANSFER_MANAGER_H

#include <string>

class FileTransferManagerResumable {
public:
    static bool upload(const std::string& localPath, const std::string& remoteUrl,
                       const std::string& keyfile = "", int maxRetries = 3);
    static bool download(const std::string& remoteUrl, const std::string& localPath,
                         const std::string& keyfile = "", int maxRetries = 3);
};

#endif
