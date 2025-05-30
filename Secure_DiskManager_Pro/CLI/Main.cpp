#include <iostream>
#include "src/main_controller.h"
#include "src/command_dispatcher.h"

void PrintHelp() {
    std::cout << R"(
⚙️  SecureDiskManagerPro v1.0 - Ultra-fast file utility suite

Usage:
  SecureDiskManagerPro [options]

Core Commands:
  --search <query>           Fast local file search
  --copy <file>              Copy file to internal buffer or target
  --encrypt <keyfile>        Encrypt input file using keyfile
  --decrypt <keyfile>        Decrypt input file using keyfile
  --compress                 Compress input file (SIMD + threaded)
  --decompress               Decompress archive
  --upload <url>             Upload to server (with resume support)
  --download <url>           Download from server (with resume)
  --retries <n>              Retry count for upload/download (default: 3)
  --shred                    Securely shred file (SIMD-enhanced)
  --selfdelete               Perform secure self-deletion

Other:
  --help, -h                 Show this help screen
)";
}

int main(int argc, char* argv[]) {
    std::cout << "⚙️  SecureDiskManagerPro v1.0\n";

    if (argc >= 2 && std::string(argv[1]).rfind("--", 0) == 0) {
        handle_stealthmailer_cli(argc, argv);
        return 0;
    }

    if (argc >= 3 && std::string(argv[1]) == "--dispatchqueue") {
        handle_dispatch_queue_cli(argv[2]);
        return 0;
    }

    if (argc == 1 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
        PrintHelp();
        return 0;
    }

    try {
        CommandContext ctx = MainController::ParseArgs(argc, argv);
        CommandDispatcher::Dispatch(ctx);
    } catch (const std::exception& ex) {
        std::cerr << "❌ Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}

ghostmode_vanisher tuneling supports:

🛠️ Notes:
This supports AES-256-CBC encryption of the traffic.

Proxy tunnel targets any SOCKS5/HTTP-compatible IP and port.

We’re sending back encrypted relays over the same TCP stream for now (e.g. simulate encrypted shell or file forward).

Future expansions could include:

Full TLS layering

VPN routing through tun/tap interface

Key rotation logic

IPv6 and UDP relay support

Firewall evasion modules