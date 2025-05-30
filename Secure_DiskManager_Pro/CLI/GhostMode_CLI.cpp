#include <iostream>
#include <string>
#include "GhostMode_Controller.h"
#include "IdentitySwitcher.h"
#include "GhostUtils.h"
#include "StealthMailer.h"
#include "SecureDeletion.h"
#include "MacSpoofer.h"
#include "PayloadDispatcher.h"
#include "json.hpp" // nlohmann::json

#include <fstream>

void handle_stealthmailer_cli(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "[Error] Not enough arguments for StealthMailer." << std::endl;
        return;
    }

    std::string cmd = argv[1];

    if (cmd == "--sendmail") {
        if (argc < 3) {
            std::cerr << "[Usage] --sendmail <payload.zip>" << std::endl;
            return;
        }

        StealthMailer::MailerConfig cfg;
        cfg.payloadPath = argv[2];
        cfg.smtpServer = "smtp.example.com";   // TODO: replace with config file or arguments
        cfg.smtpUser = "unused";               // Loaded from encrypted XML
        cfg.smtpPassword = "unused";
        cfg.toAddress = "dest@example.com";
        cfg.useTor = true;
        cfg.requireVPN = true;
        cfg.spoofMac = true;
        cfg.shredAfterSend = true;

        if (!StealthMailer::send_payload_securely(cfg)) {
            std::cerr << "[FAIL] Secure send failed." << std::endl;
        }

    } else if (cmd == "--spoofmac") {
        MacSpoofer::spoof_mac_random();

    } else if (cmd == "--checktor") {
        if (!StealthMailer::verify_tor_connection()) {
            std::cerr << "[TOR] Verification failed." << std::endl;
        } else {
            std::cout << "[TOR] Connection verified." << std::endl;
        }

    } else if (cmd == "--shred") {
        if (argc < 3) {
            std::cerr << "[Usage] --shred <file>" << std::endl;
            return;
        }
        SecureDeletion::shred_file(argv[2]);

    } else {
        std::cerr << "[Error] Unknown StealthMailer command: " << cmd << std::endl;
    }
}

void handle_dispatch_queue_cli(const std::string& queuePath) {
    std::ifstream in(queuePath);
    if (!in.is_open()) {
        std::cerr << "[Queue] Failed to open queue file: " << queuePath << std::endl;
        return;
    }

    nlohmann::json j;
    in >> j;

    for (auto& item : j["jobs"]) {
        PayloadDispatcher::PayloadJob job;
        job.method = item["method"];
        job.filePath = item["file"];
        job.remoteName = item["remote_name"];
        job.target = item["target"];
        job.user = item["user"];
        job.password = item["password"];
        PayloadDispatcher::queue_payload(job);
    }

    bool useTor = j.value("use_tor", true);
    bool spoofMac = j.value("spoof_mac", true);
    bool shred = j.value("shred_after", true);

    PayloadDispatcher::dispatch_all(useTor, spoofMac, shred);
}

void printHelp() {
    std::cout << "\nGhostMode Vanisher - CLI Mode\n"
              << "Usage:\n"
              << "  --switch-identity NAME   Switch to the specified identity\n"
              << "  --list-identities        Show all available identities\n"
              << "  --help                   Show this help screen\n\n";
}

int main(int argc, char** argv) {
    const std::string baseIdentityPath = "./ghost_identities";

    IdentitySwitcher switcher(baseIdentityPath);
    GhostMode_Controller controller;

    if (argc == 1) {
        printHelp();
        return 0;
    }

    std::string cmd = argv[1];

    if (cmd == "--switch-identity") {
        if (argc < 3) {
            std::cerr << "Error: Missing identity name.\n";
            return 1;
        }
        std::string identity = argv[2];
        if (!switcher.switchToIdentity(identity)) {
            std::cerr << "Error: Failed to switch identity.\n";
            return 1;
        }
    } else if (cmd == "--list-identities") {
        auto identities = switcher.listIdentities();
        std::cout << "Available Identities:\n";
        for (const auto& name : identities) {
            std::cout << "  " << name << "\n";
        }
    } else if (cmd == "--help") {
        printHelp();
    } else {
        std::cerr << "Unknown command. Use --help to see available options.\n";
        return 1;
    }

    return 0;
}
