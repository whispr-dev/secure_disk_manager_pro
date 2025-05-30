#include "Ghost_UI_CLI.h"
#include <iostream>
#include <sstream>

Ghost_UI_CLI::Ghost_UI_CLI() {}

void Ghost_UI_CLI::showBanner() const {
    std::cout << R"(
   ________               __    ______          __         
  / ____/ /_  ___  ____  / /__ / ____/__  _____/ /__  _____
 / /   / __ \/ _ \/ __ \/ / _ \\__ \/ _ \/ ___/ / _ \/ ___/
/ /___/ / / /  __/ /_/ / /  __/__/ /  __/ /__/ /  __/ /    
\____/_/ /_/\___/ .___/_/\___/____/\___/\___/_/\___/_/     
               /_/                                        
)" << std::endl;
}

void Ghost_UI_CLI::showMenu() {
    std::string input;
    while (true) {
        std::cout << "\nghost> ";
        std::getline(std::cin, input);
        if (input == "exit") break;
        handleUserCommand(input);
    }
}

void Ghost_UI_CLI::handleUserCommand(const std::string& input) {
    if (input == "help") {
        printHelp();
    } else if (input == "commands") {
        listCommands();
    } else {
        executeCommand(input);
    }
}

void Ghost_UI_CLI::printHelp() const {
    std::cout << "Type 'commands' to see a list of available commands.\n"
                 "Type 'exit' to quit the shell.\n";
}

void Ghost_UI_CLI::listCommands() const {
    std::cout << "- help\n"
              << "- commands\n"
              << "- rotate_identity\n"
              << "- shred\n"
              << "- encrypt\n"
              << "- decrypt\n"
              << "- compress\n"
              << "- upload\n"
              << "- selfdelete\n";
}

void Ghost_UI_CLI::executeCommand(const std::string& command) {
    std::cout << "[!] Executing stub: " << command << "\n";
    // Actual dispatch will call CommandDispatcher or GhostMode_Controller methods
}
