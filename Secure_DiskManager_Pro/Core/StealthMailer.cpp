#include "StealthMailer.h"
#include "SecureDeletion.h"
#include "MacSpoofer.h"
#include "GhostVPNManager.h"
#include "FileUploader.h"
#include "GhostStealthNet.h"

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace StealthMailer {

    bool verify_tor_connection() {
        // Stub: add real implementation via SOCKS5 request or libcurl proxy check
        std::cout << "[TOR] Verifying Tor connection... (stubbed)" << std::endl;
        return true;
    }

    bool decrypt_credentials(const std::string& encFilePath, std::string& outUsername, std::string& outPassword) {
        // Stub: Replace with DPAPI or AES decrypt logic to extract creds
        std::cout << "[Creds] Decrypting: " << encFilePath << " (stubbed)" << std::endl;
        outUsername = "stealth@example.com";
        outPassword = "supersecret";
        return true;
    }

    bool send_payload_securely(const MailerConfig& config) {
        std::cout << "[Mailer] Starting secure send routine..." << std::endl;

        if (config.requireVPN && !GhostVPNManager::is_vpn_active()) {
            std::cout << "[VPN] Not active, attempting to start..." << std::endl;
            if (!GhostVPNManager::start_vpn_connection("stealth")) {
                std::cerr << "[VPN] Failed to start VPN connection." << std::endl;
                return false;
            }
        }

        if (config.useTor && !verify_tor_connection()) {
            std::cerr << "[TOR] Tor connection failed!" << std::endl;
            return false;
        }

        if (config.spoofMac) {
            std::cout << "[MAC] Spoofing MAC address..." << std::endl;
            MacSpoofer::spoof_mac_random();
        }

        std::string user, pass;
        if (!decrypt_credentials("resources/mail_ic_enc.xml", user, pass)) {
            std::cerr << "[Creds] Failed to decrypt credentials." << std::endl;
            return false;
        }

        std::cout << "[SEND] Uploading payload..." << std::endl;
        if (!FileUploader::send_via_smtp_tor(config.payloadPath, config.smtpServer, user, pass, config.toAddress)) {
            std::cerr << "[SEND] Payload failed to send!" << std::endl;
            return false;
        }

        if (config.shredAfterSend) {
            std::cout << "[SHRED] Shredding payload..." << std::endl;
            SecureDeletion::shred_file(config.payloadPath);
        }

        std::cout << "[DONE] Payload successfully sent with stealth!" << std::endl;
        return true;
    }

} // namespace StealthMailer
