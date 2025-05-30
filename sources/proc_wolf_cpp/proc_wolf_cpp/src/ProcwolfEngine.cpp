#include <iostream>
#include <fstream>
#include <filesystem>
#include <psapi.h>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <sqlite3.h>

namespace fs = std::filesystem;

ProcWolfEngine::ProcWolfEngine(const std::string& dbPath, const std::string& whitelistPath)
    : dbFile(dbPath), whitelistFile(whitelistPath) {
    loadWhitelist();
}

void ProcWolfEngine::loadWhitelist() {
    std::ifstream infile(whitelistFile);
    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (fs::exists(line)) {
            whitelistedPaths.push_back(line);
        } else {
            whitelistedNames.push_back(line);
        }
    }
}

bool ProcWolfEngine::isWhitelisted(const std::string& name, const std::string& path) const {
    for (const auto& n : whitelistedNames)
        if (_stricmp(name.c_str(), n.c_str()) == 0) return true;
    for (const auto& p : whitelistedPaths)
        if (path.find(p) != std::string::npos) return true;
    return false;
}

std::vector<ProcessInfo> ProcWolfEngine::listProcesses(bool assessThreat) {
    std::vector<ProcessInfo> results;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return results;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(snapshot, &pe)) {
        do {
            DWORD pid = pe.th32ProcessID;
            HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
            if (hProc) {
                char exePath[MAX_PATH] = "<unknown>";
                if (GetModuleFileNameExA(hProc, NULL, exePath, MAX_PATH)) {
                    ProcessInfo info;
                    info.pid = pid;
                    info.name = pe.szExeFile;
                    info.exePath = exePath;
                    info.username = "N/A";  // TODO: Get user
                    info.cpuPercent = 0.0;  // TODO: CPU usage
                    info.trusted = isWhitelisted(info.name, info.exePath);
                    info.threatLevel = assessThreat ? assessThreatLevel(info) : 0;
                    results.push_back(info);
                }
                CloseHandle(hProc);
            }
        } while (Process32Next(snapshot, &pe));
    }
    CloseHandle(snapshot);
    return results;
}

std::optional<ProcessInfo> ProcWolfEngine::getProcessByPID(int pid) {
    for (const auto& proc : listProcesses(true)) {
        if (proc.pid == pid) return proc;
    }
    return std::nullopt;
}

std::vector<ProcessInfo> ProcWolfEngine::getProcessesByName(const std::string& name) {
    std::vector<ProcessInfo> matches;
    for (const auto& proc : listProcesses(true)) {
        if (_stricmp(proc.name.c_str(), name.c_str()) == 0)
            matches.push_back(proc);
    }
    return matches;
}

bool ProcWolfEngine::killProcess(int pid, bool force) {
    DWORD access = force ? PROCESS_TERMINATE : PROCESS_TERMINATE;
    HANDLE hProc = OpenProcess(access, FALSE, pid);
    if (!hProc) return false;
    BOOL result = TerminateProcess(hProc, 0);
    CloseHandle(hProc);
    return result;
}

bool ProcWolfEngine::nukeProcess(int pid) {
    // TODO: Implement artifact deletion, service unregister, registry clean
    return killProcess(pid, true);
}

void ProcWolfEngine::logHistory(const ProcessInfo& info, const std::string& action) {
    sqlite3* db;
    if (sqlite3_open(dbFile.c_str(), &db) == SQLITE_OK) {
        std::string sql = "INSERT INTO process_log (pid, name, path, action, timestamp) VALUES (?, ?, ?, ?, strftime('%s','now'));";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, info.pid);
            sqlite3_bind_text(stmt, 2, info.name.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, info.exePath.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, action.c_str(), -1, SQLITE_STATIC);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }
}

int ProcWolfEngine::assessThreatLevel(const ProcessInfo& proc) const {
    if (proc.trusted) return 0;
    if (proc.exePath.find("AppData") != std::string::npos) return 3;
    if (proc.name.length() > 30 || proc.name.find("_") != std::string::npos) return 2;
    return 1;
}