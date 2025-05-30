#ifndef GHOST_UI_CLI_H
#define GHOST_UI_CLI_H

#include <string>
#include <vector>

class Ghost_UI_CLI {
public:
    Ghost_UI_CLI();
    void showBanner() const;
    void showMenu();
    void handleUserCommand(const std::string& input);

private:
    void printHelp() const;
    void listCommands() const;
    void executeCommand(const std::string& command);
};

#endif // GHOST_UI_CLI_H
