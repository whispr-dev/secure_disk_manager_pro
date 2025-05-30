#include <iostream>
#include <string>
#include <algorithm>

void print_process(const ProcessInfo& p, bool showThreat) {
    std::cout << "PID: " << p.pid
              << " | Name: " << p.name
              << " | Path: " << p.exePath;
    if (showThreat) {
        std::cout << " | Threat: " << p.threatLevel;
        if (p.trusted) std::cout << " (Trusted)";
    }
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage:\n";
        std::cout << "  ProcWolfCLI list [--assess]\n";
        std::cout << "  ProcWolfCLI kill <pid|name> [--force]\n";
        std::cout << "  ProcWolfCLI nuke <pid|name>\n";
        return 1;
    }

    std::string command = argv[1];
    ProcWolfEngine engine("logs/procwolf.db", "whitelist/proc-wofl_whitelist.txt");

    if (command == "list") {
        bool assess = (argc > 2 && std::string(argv[2]) == "--assess");
        auto procs = engine.listProcesses(assess);
        for (const auto& proc : procs)
            print_process(proc, assess);
        std::cout << "Total: " << procs.size() << "\n";
    }
    else if (command == "kill" && argc >= 3) {
        std::string target = argv[2];
        bool force = (argc > 3 && std::string(argv[3]) == "--force");

        try {
            int pid = std::stoi(target);
            if (engine.killProcess(pid, force)) {
                std::cout << "Killed PID " << pid << "\n";
            } else {
                std::cerr << "Failed to kill PID " << pid << "\n";
            }
        } catch (...) {
            auto procs = engine.getProcessesByName(target);
            if (procs.empty()) {
                std::cerr << "No process named '" << target << "' found.\n";
            }
            for (const auto& proc : procs) {
                if (engine.killProcess(proc.pid, force)) {
                    std::cout << "Killed " << proc.name << " (PID: " << proc.pid << ")\n";
                } else {
                    std::cerr << "Failed to kill PID " << proc.pid << "\n";
                }
            }
        }
    }
    else if (command == "nuke" && argc >= 3) {
        std::string target = argv[2];

        try {
            int pid = std::stoi(target);
            if (engine.nukeProcess(pid)) {
                std::cout << "Nuked PID " << pid << "\n";
            } else {
                std::cerr << "Failed to nuke PID " << pid << "\n";
            }
        } catch (...) {
            auto procs = engine.getProcessesByName(target);
            if (procs.empty()) {
                std::cerr << "No process named '" << target << "' found.\n";
            }
            for (const auto& proc : procs) {
                if (engine.nukeProcess(proc.pid)) {
                    std::cout << "Nuked " << proc.name << " (PID: " << proc.pid << ")\n";
                } else {
                    std::cerr << "Failed to nuke PID " << proc.pid << "\n";
                }
            }
        }
    }
    else {
        std::cerr << "Unknown command or bad arguments.\n";
        return 1;
    }

    return 0;
}