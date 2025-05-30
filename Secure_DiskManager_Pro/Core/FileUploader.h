#ifndef FILE_UPLOADER_H
#define FILE_UPLOADER_H

#include <string>

namespace FileUploader {

    // Send file over SMTP (supports SOCKS5 proxy for TOR)
    bool send_via_smtp_tor(
        const std::string& filePath,
        const std::string& smtpServer,
        const std::string& smtpUser,
        const std::string& smtpPassword,
        const std::string& toAddress
    );

    bool send_via_ftp_tor(
        const std::string& filePath,
        const std::string& ftpServer,
        const std::string& ftpUser,
        const std::string& ftpPassword,
        const std::string& remoteFilename
    );

}

#endif // FILE_UPLOADER_H
