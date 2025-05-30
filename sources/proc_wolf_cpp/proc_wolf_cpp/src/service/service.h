#define PROCWOLF_SERVICE_H

#include <windows.h>
#include <string>

class ProcWolfService {
public:
    static void Install(const std::wstring& serviceName);
    static void Uninstall(const std::wstring& serviceName);
    static void Run(const std::wstring& serviceName);

private:
    static void WINAPI ServiceMain(DWORD argc, LPWSTR* argv);
    static void WINAPI ServiceCtrlHandler(DWORD ctrlCode);

    static void MonitorLoop();
    static void ReportServiceStatus(DWORD currentState, DWORD win32ExitCode, DWORD waitHint);

    static SERVICE_STATUS_HANDLE serviceStatusHandle;
    static SERVICE_STATUS serviceStatus;
    static bool running;
};
