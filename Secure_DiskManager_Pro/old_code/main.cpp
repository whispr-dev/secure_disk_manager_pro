// Secure_DiskManager_Pro main.cpp - a high security file management tool
#include <iostream>
#include <string>

#include "UltraFastSearch.h"
#include "SecureDeletion_SIMD.h"
#include "FileEncryption.h"
#include "DiskManagement.h"
#include "SecureDeletion.h"
#include "SelfCleanup.h"
#include "ServiceManager.h"
#include "SystemMonitor.h"

void printUsage() {
    std::cout << "Usage:\n"
              << "  --encrypt <file> <password>\n"
              << "  --decrypt <file> <password>\n"
              << "  --encrypt <file> --keyfile <keyfile>\n"
              << "  --decrypt <file> --keyfile <keyfile>\n"
              << "  --shred <file>\n"
              << "  --diskusage <path>\n"
              << "  --selfdelete\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];
    FileEncryption fe;

    if (command == "--encrypt" && argc == 4) {
        std::string file = argv[2];
        std::string password = argv[3];
        if (fe.encryptFile(file, password)) {
            std::cout << "File encrypted successfully.\n";
        } else {
            std::cerr << "Encryption failed.\n";
        }
    } else if (command == "--decrypt" && argc == 4) {
        std::string file = argv[2];
        std::string password = argv[3];
        if (fe.decryptFile(file, password)) {
            std::cout << "File decrypted successfully.\n";
        } else {
            std::cerr << "Decryption failed.\n";
        }
    } else if (command == "--encrypt" && std::string(argv[3]) == "--keyfile" && argc == 5) {
        std::string file = argv[2];
        std::string keyfile = argv[4];
        if (fe.encryptFileWithKeyfile(file, keyfile)) {
            std::cout << "File encrypted with keyfile successfully.\n";
        } else {
            std::cerr << "Keyfile encryption failed.\n";
        }
    } else if (command == "--decrypt" && std::string(argv[3]) == "--keyfile" && argc == 5) {
        std::string file = argv[2];
        std::string keyfile = argv[4];
        if (fe.decryptFileWithKeyfile(file, keyfile)) {
            std::cout << "File decrypted with keyfile successfully.\n";
        } else {
            std::cerr << "Keyfile decryption failed.\n";
        }
    } else if (command == "--shred" && argc == 3) {
        std::string file = argv[2];
        if (SecureDeletion::shredFile(file)) {
            std::cout << "File securely shredded: " << file << "\n";
        } else {
            std::cerr << "Failed to shred file.\n";
        }
    } else if (command == "--diskusage" && argc == 3) {
        std::string path = argv[2];
        uintmax_t used = DiskManagement::getDiskUsage(path);
        std::cout << "Disk usage for " << path << ": " << used << " bytes used.\n";
    } else if (command == "--selfdelete") {
        std::string confirm;
        std::cout << "Are you absolutely sure you want this tool to delete itself? Type YES to confirm: ";
        std::cin >> confirm;
        if (confirm == "YES") {
            SelfCleanup::run();
        } else {
            std::cout << "Self-deletion cancelled.\n";
        }
    else if (command == "--services" && argc == 3) {
        std::string action = argv[2];
    if (action == "list") {
        ServiceManager::listServices();
    }
    } else if (command == "--services" && argc == 4) {
        std::string action = argv[2];
        std::string target = argv[3];
        if (action == "start") {
            if (ServiceManager::startService(target))
                std::cout << "Service started.\n";
            else std::cerr << "Failed to start.\n";
        } else if (action == "stop") {
            if (ServiceManager::stopService(target))
                std::cout << "Service stopped.\n";
            else std::cerr << "Failed to stop.\n";
        }
    } else if (command == "--sysinfo") {
        unsigned long long total = 0, free = 0;
        SystemMonitor::getMemoryStats(total, free);
        std::cout << "Total RAM: " << total / (1024 * 1024) << " MB\n";
        std::cout << "Free RAM:  " << free / (1024 * 1024) << " MB\n";
        std::cout << "CPU Usage: " << SystemMonitor::getCpuUsage() << " %\n";
    } else {
        printUsage();
        return 1;
    else if (command == "--randint" && argc == 4) {
        uint32_t min = std::stoi(argv[2]);
        uint32_t max = std::stoi(argv[3]);
        std::cout << RNG::getRandomInt(min, max) << "\n";
    } else if (command == "--randbytes" && argc == 3) {
        size_t size = std::stoul(argv[2]);
        auto data = RNG::getRandomBytes(size);
        for (uint8_t b : data) std::cout << std::hex << (int)b << " ";
        std::cout << "\n";
    else if (command == "--genkey" && argc == 3) {
        RNG_SIMD::initialize();
        RNG_SIMD::writeKeyfile(argv[2]);
    } else if (command == "--shredsimd" && argc == 3) {
        SecureDeletion_SIMD::shredFile(argv[2]);
     #include "KeyManager.h"
    else if (command == "--keygen" && argc == 3) {
        if (KeyManager::generateKeyfile(argv[2]))
            std::cout << "Keyfile generated.\n";
        else
            std::cerr << "Keyfile generation failed.\n";
    } else if (command == "--keylist") {
        KeyManager::listKeyfiles();
    } else if (command == "--keydel" && argc == 3) {
        if (KeyManager::deleteKeyfile(argv[2]))
            std::cout << "Keyfile deleted.\n";
        else
            std::cerr << "Keyfile deletion failed.\n";
    else if (command == "--shredsimd" && argc == 3) {
        if (SecureDeletion_SIMD::shredFile(argv[2]))
            std::cout << "SIMD secure shred complete: " << argv[2] << "\n";
        else
            std::cerr << "Failed to shred file with SIMD: " << argv[2] << "\n";
    else if (command == "--search" && argc == 3) {
        auto results = UltraFastSearch::search(argv[2]);
        UltraFastSearch::printResults(results);
    else if (command == "--search" && (argc == 3 || argc == 4)) {
        std::string pattern = argv[2];
        std::string root = (argc == 4) ? argv[3] : "C:\\";
        auto results = UltraFastSearch::search(pattern, root);
        UltraFastSearch::printResults(results);
    }
    else if (command == "--searchcsv" && argc == 5) {
        std::string pattern = argv[2];
        std::string root = argv[3];
        std::string out = argv[4];
        auto results = UltraFastSearch::search(pattern, root);
        if (UltraFastSearch::exportToCSV(results, out)) {
            std::cout << "Exported search results to " << out << "\n";
        } else {
            std::cerr << "Failed to write CSV to " << out << "\n";
    else if (command == "--ultrasearch" && argc >= 3) {
        std::string pattern = argv[2];
        std::string root = (argc >= 4) ? argv[3] : "C:\\";
        bool useEntropy = (argc >= 5 && std::string(argv[4]) == "--entropy");

        auto results = SearchEngine::findMatchingFiles(root, pattern, useEntropy);
        SearchEngine::printResults(results);
                                    }
                                }
                            }
                        }
                    }
                }   
            }
        }
    }
}

    return 0;
}
