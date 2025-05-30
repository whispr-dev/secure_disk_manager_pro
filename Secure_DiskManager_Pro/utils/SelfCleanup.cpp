#include "SelfCleanup.h"
#include <windows.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <thread>
#include <chrono>

void SelfCleanup::run() {
    char exePath[MAX_PATH];
    if (!GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
        MessageBoxA(NULL, "Failed to get executable path.", "SelfDelete Error", MB_ICONERROR);
        return;
    }

    std::string exeFile = exePath;
    std::string batchFile = exeFile + ".del.bat";

    // Create .bat file that waits and deletes the .exe
    std::ofstream bat(batchFile);
    if (!bat.is_open()) {
        MessageBoxA(NULL, "Failed to write batch file.", "SelfDelete Error", MB_ICONERROR);
        return;
    }

    bat << "@echo off\n";
    bat << "ping 127.0.0.1 -n 3 > nul\n";  // ~2 second delay
    bat << ":repeat\n";
    bat << "del \"" << exeFile << "\" > nul\n";
    bat << "if exist \"" << exeFile << "\" goto repeat\n";
    bat << "del \"%~f0\"\n";  // Delete the batch file itself
    bat.close();

    // Execute .bat file minimized and exit
    ShellExecuteA(NULL, "open", batchFile.c_str(), NULL, NULL, SW_HIDE);

    // Exit process to release the .exe file
    ExitProcess(0);
}
