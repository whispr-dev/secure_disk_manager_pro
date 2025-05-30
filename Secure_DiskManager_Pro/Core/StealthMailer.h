#ifndef STEALTH_MAILER_H
#define STEALTH_MAILER_H

#include <string>

namespace StealthMailer {

    struct MailerConfig {
        std::string payloadPath;
        std::string smtpServer;
        std::string smtpUser;
        std::string smtpPassword;
        std::string toAddress;
        bool useTor;
        bool spoofMac;
        bool shredAfterSend;
        bool requireVPN;
    };

    bool send_payload_securely(const MailerConfig& config);
    bool verify_tor_connection();
    bool decrypt_credentials(const std::string& encFilePath, std::string& outUsername, std::string& outPassword);

} // namespace StealthMailer

#endif // STEALTH_MAILER_H
