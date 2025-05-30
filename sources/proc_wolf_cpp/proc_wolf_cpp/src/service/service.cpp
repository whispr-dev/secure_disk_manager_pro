#include "../core/ProcWolfEngine.h"
#include <iostream>
#include <thread>
#include <chrono>

SERVICE_STATUS_HANDLE ProcWolfService::serviceStatusHandle = nullptr;
SERVICE_STATUS ProcWolfService::serviceStatus = {};
bool ProcWolfService::running = false;

void ProcWolfService::Install(const std::wstring& serviceName) {
    SC_HANDLE hSCM = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (!hSCM) {
        std::wcerr << L"Failed to open SCM.\n";
        return;
    }

    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);

    SC_HANDLE hService = CreateServiceW(
        hSCM, serviceName.c_str(), serviceName.c_str(),
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
        path, nullptr, nullptr, nullptr, nullptr, nullptr);

    if (!hService) {
        std::wcerr << L"Service install failed.\n";
    } else {
        std::wcout << L"Service installed.\n";
        CloseServiceHandle(hService);
    }
    CloseServiceHandle(hSCM);
}

void ProcWolfService::Uninstall(const std::wstring& serviceName) {
    SC_HANDLE hSCM = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!hSCM) {
        std::wcerr << L"Failed to open SCM.\n";
        return;
    }

    SC_HANDLE hService = OpenServiceW(hSCM, serviceName.c_str(), DELETE);
    if (!hService) {
        std::wcerr << L"Failed to open service.\n";
    } else {
        if (DeleteService(hService)) {
            std::wcout << L"Service uninstalled.\n";
        } else {
            std::wcerr << L"Service deletion failed.\n";
        }
        CloseServiceHandle(hService);
    }
    CloseServiceHandle(hSCM);
}

void ProcWolfService::Run(const std::wstring& serviceName) {
    SERVICE_TABLE_ENTRYW serviceTable[] = {
        { const_cast<LPWSTR>(serviceName.c_str()), ServiceMain },
        { nullptr, nullptr }
    };
    StartServiceCtrlDispatcherW(serviceTable);
}

void WINAPI ProcWolfService::ServiceMain(DWORD, LPWSTR*) {
    serviceStatusHandle = RegisterServiceCtrlHandlerW(L"ProcWolfService", ServiceCtrlHandler);
    if (!serviceStatusHandle) return;

    ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    running = true;
    ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);

    MonitorLoop();

    ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

void WINAPI ProcWolfService::ServiceCtrlHandler(DWORD ctrlCode) {
    switch (ctrlCode) {
        case SERVICE_CONTROL_STOP:
            ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
            running = false;
            break;
        default:
            break;
    }
}

void ProcWolfService::ReportServiceStatus(DWORD currentState, DWORD win32ExitCode, DWORD waitHint) {
    serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    serviceStatus.dwCurrentState = currentState;
    serviceStatus.dwWin32ExitCode = win32ExitCode;
    serviceStatus.dwWaitHint = waitHint;
    serviceStatus.dwControlsAccepted = (currentState == SERVICE_RUNNING) ? SERVICE_ACCEPT_STOP : 0;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);
}

void ProcWolfService::MonitorLoop() {
    ProcWolfEngine engine("logs/procwolf.db", "whitelist/proc-wofl_whitelist.txt");
    while (running) {
        auto procs = engine.listProcesses(true);
        for (const auto& p : procs) {
            if (p.threatLevel >= 3 && !p.trusted) {
                engine.killProcess(p.pid, true);
                engine.logHistory(p, "Auto-killed by service");
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}