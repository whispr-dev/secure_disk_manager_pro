#include "GhostMode_Controller.h"
#include <iostream>
#include <string>

void PrintHelp() {
    std::cout << "GhostCLI Command Options:\n"
              << "  --help                     Show this help message\n"
              << "  --init                     Initialize GhostMode\n"
              << "  --run                      Run full secure GhostMode session\n"
              << "  --identity rotate          Rotate current identity/persona\n"
              << "  --tunnel start <endpoint> Start outbound tunnel to endpoint (e.g., proxy:443)\n"
              << "  --tunnel stop              Stop active tunnel\n";
}

int main(int argc, char* argv[]) {
    GhostMode_Controller controller;

    if (argc < 2) {
        PrintHelp();
        return 1;
    }

    std::string cmd = argv[1];

    if (cmd == "--help") {
        PrintHelp();
    } else if (cmd == "--init") {
        controller.Init();
    } else if (cmd == "--run") {
        controller.Init();
        controller.Run();
    } else if (cmd == "--identity" && argc >= 3 && std::string(argv[2]) == "rotate") {
        controller.RotateIdentity();
    } else if (cmd == "--tunnel") {
        if (argc >= 3 && std::string(argv[2]) == "start" && argc == 4) {
            controller.Init(); // Required for tunnel to work
            controller.StartTunneling(argv[3]);
        } else if (argc >= 3 && std::string(argv[2]) == "stop") {
            controller.Init(); // Make sure tunnel system is loaded
            controller.StopTunneling();
        } else {
            std::cerr << "Invalid tunnel command. Use --tunnel start <endpoint> or --tunnel stop\n";
        }
    } else {
        std::cerr << "Unknown command.\n";
        PrintHelp();
        return 1;
    }

    return 0;
}
