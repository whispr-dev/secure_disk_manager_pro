#include "Ghost_Tunnel.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

GhostTunnel::GhostTunnel() : proxyPort_(0), proxySocket_(-1), running_(false) {}

GhostTunnel::~GhostTunnel() {
    stopTunnel();
}

bool GhostTunnel::initialize(const std::string& proxyAddress, int proxyPort, const std::string& encryptionKey) {
    proxyAddress_ = proxyAddress;
    proxyPort_ = proxyPort;
    encryptionKey_ = encryptionKey;
    return true;
}

bool GhostTunnel::startTunnel() {
    if (running_) return false;
    running_ = true;
    tunnelThread_ = std::thread(&GhostTunnel::tunnelLoop, this);
    return true;
}

void GhostTunnel::stopTunnel() {
    if (!running_) return;
    running_ = false;
    if (tunnelThread_.joinable()) {
        tunnelThread_.join();
    }
    closeSocket();
}

bool GhostTunnel::isRunning() const {
    return running_;
}

void GhostTunnel::tunnelLoop() {
    if (!connectToProxy()) {
        std::cerr << "[GhostTunnel] Failed to connect to proxy.\n";
        running_ = false;
        return;
    }

    std::cout << "[GhostTunnel] Tunnel established to " << proxyAddress_ << ":" << proxyPort_ << "\n";
    encryptAndRelayTraffic();
}

bool GhostTunnel::connectToProxy() {
    return setupSocket();
}

bool GhostTunnel::setupSocket() {
    proxySocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (proxySocket_ < 0) {
        perror("socket");
        return false;
    }

    sockaddr_in serverAddr {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(proxyPort_);
    inet_pton(AF_INET, proxyAddress_.c_str(), &serverAddr.sin_addr);

    if (connect(proxySocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect");
        close(proxySocket_);
        return false;
    }

    return true;
}

void GhostTunnel::closeSocket() {
    if (proxySocket_ != -1) {
        close(proxySocket_);
        proxySocket_ = -1;
    }
}

void GhostTunnel::encryptAndRelayTraffic() {
    constexpr size_t bufSize = 4096;
    unsigned char buffer[bufSize];
    unsigned char ciphertext[bufSize + AES_BLOCK_SIZE];
    int outlen = 0;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    const EVP_CIPHER* cipher = EVP_aes_256_cbc();
    unsigned char key[32], iv[16];

    // Derive key and IV from encryptionKey_
    std::memset(key, 0, sizeof(key));
    std::memset(iv, 0, sizeof(iv));
    std::strncpy((char*)key, encryptionKey_.c_str(), std::min(encryptionKey_.size(), sizeof(key)));

    EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv);

    while (running_) {
        ssize_t bytesRead = recv(proxySocket_, buffer, bufSize, 0);
        if (bytesRead <= 0) break;

        EVP_EncryptUpdate(ctx, ciphertext, &outlen, buffer, bytesRead);
        send(proxySocket_, ciphertext, outlen, 0);
    }

    EVP_CIPHER_CTX_free(ctx);
    std::cout << "[GhostTunnel] Tunnel session ended.\n";
}
