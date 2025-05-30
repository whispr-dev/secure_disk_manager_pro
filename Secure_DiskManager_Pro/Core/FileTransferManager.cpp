#include "FileTransferManager.h"
#include "../Crypto.h"
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>

static std::vector<uint8_t> loadKey(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)), {});
}

static void xorCrypt(std::vector<uint8_t>& data, const std::vector<uint8_t>& key) {
    for (size_t i = 0; i < data.size(); ++i)
        data[i] ^= key[i % key.size()];
}

bool FileTransferManager::upload(const std::string& localPath, const std::string& remoteUrl,
                                          const std::string& keyfile, int maxRetries) {
    if (!std::filesystem::exists(localPath)) return false;
    std::ifstream in(localPath, std::ios::binary);
    std::vector<uint8_t> fileData((std::istreambuf_iterator<char>(in)), {});
    if (!keyfile.empty()) xorCrypt(fileData, loadKey(keyfile));

    size_t offset = 0;
    std::string resumePath = localPath + ".upload_resume";
    std::ifstream resumeFile(resumePath);
    if (resumeFile) resumeFile >> offset;

    for (int attempt = 0; attempt < maxRetries; ++attempt) {
        CURL* curl = curl_easy_init();
        if (!curl) return false;

        struct UploadCtx { const std::vector<uint8_t>* data; size_t offset; } ctx { &fileData, offset };

        curl_easy_setopt(curl, CURLOPT_URL, remoteUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(fileData.size() - offset));

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, [](char* buffer, size_t size, size_t nitems, void* userdata) -> size_t {
            UploadCtx* ctx = static_cast<UploadCtx*>(userdata);
            size_t remain = ctx->data->size() - ctx->offset;
            size_t tocopy = std::min(size * nitems, remain);
            memcpy(buffer, ctx->data->data() + ctx->offset, tocopy);
            ctx->offset += tocopy;
            return tocopy;
        });

        curl_easy_setopt(curl, CURLOPT_READDATA, &ctx);
        curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, static_cast<curl_off_t>(offset));

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            std::remove(resumePath.c_str());
            return true;
        }

        std::ofstream out(resumePath); out << ctx.offset; out.close();
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return false;
}

static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    auto out = reinterpret_cast<std::pair<std::ofstream*, size_t>*>(userp);
    out->first->write(static_cast<char*>(contents), size * nmemb);
    out->second += size * nmemb;
    return size * nmemb;
}

bool FileTransferManager::download(const std::string& remoteUrl, const std::string& localPath,
                                            const std::string& keyfile, int maxRetries) {
    size_t offset = 0;
    std::string resumePath = localPath + ".download_resume";
    std::ofstream outFile(localPath, std::ios::binary | std::ios::app);

    if (std::filesystem::exists(resumePath)) {
        std::ifstream resumeFile(resumePath);
        resumeFile >> offset;
    } else {
        offset = std::filesystem::exists(localPath) ? std::filesystem::file_size(localPath) : 0;
    }

    for (int attempt = 0; attempt < maxRetries; ++attempt) {
        CURL* curl = curl_easy_init();
        if (!curl) return false;

        std::pair<std::ofstream*, size_t> ctx = { &outFile, offset };

        curl_easy_setopt(curl, CURLOPT_URL, remoteUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
        curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, static_cast<curl_off_t>(offset));

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            outFile.close();
            std::remove(resumePath.c_str());
            if (!keyfile.empty()) {
                std::ifstream full(localPath, std::ios::binary);
                std::vector<uint8_t> content((std::istreambuf_iterator<char>(full)), {});
                xorCrypt(content, loadKey(keyfile));
                std::ofstream over(localPath, std::ios::binary | std::ios::trunc);
                over.write(reinterpret_cast<const char*>(content.data()), content.size());
            }
            return true;
        }

        std::ofstream out(resumePath); out << ctx.second; out.close();
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return false;
}
