engine_h = r'''#ifndef PROCWOLF_ENGINE_H
#define PROCWOLF_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

struct ProcessInfo {
    int pid;
    std::string name;
    std::string exePath;
    std::string username;
    double cpuPercent;
    int threatLevel;
    bool trusted;
};

class ProcWolfEngine {
public:
    ProcWolfEngine(const std::string& dbPath, const std::string& whitelistPath);
    
    void loadWhitelist();
    bool isWhitelisted(const std::string& name, const std::string& path) const;

    std::vector<ProcessInfo> listProcesses(bool assessThreat = false);
    std::optional<ProcessInfo> getProcessByPID(int pid);
    std::vector<ProcessInfo> getProcessesByName(const std::string& name);

    bool killProcess(int pid, bool force = false);
    bool nukeProcess(int pid);

    void logHistory(const ProcessInfo& info, const std::string& action);

private:
    std::string dbFile;
    std::string whitelistFile;
    std::vector<std::string> whitelistedNames;
    std::vector<std::string> whitelistedPaths;

    int assessThreatLevel(const ProcessInfo& proc) const;
};