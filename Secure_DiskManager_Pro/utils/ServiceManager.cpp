// Service Manager
#include "ServiceManager.h"
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

void ServiceManager::listServices() {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!scm) {
        std::cerr << "Failed to open Service Control Manager.\n";
        return;
    }

    DWORD bytesNeeded = 0, servicesReturned = 0, resumeHandle = 0;
    EnumServicesStatusEx(
        scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32,
        SERVICE_STATE_ALL, NULL, 0, &bytesNeeded,
        &servicesReturned, &resumeHandle, NULL
    );

    std::vector<BYTE> buffer(bytesNeeded);
    LPENUM_SERVICE_STATUS_PROCESS services = reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESS>(buffer.data());

    if (!EnumServicesStatusEx(
            scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32,
            SERVICE_STATE_ALL, buffer.data(), buffer.size(),
            &bytesNeeded, &servicesReturned, &resumeHandle, NULL)) {
        std::cerr << "Failed to enumerate services.\n";
        CloseServiceHandle(scm);
        return;
    }

    for (DWORD i = 0; i < servicesReturned; ++i) {
        std::cout << services[i].lpServiceName << " [" << services[i].lpDisplayName << "] - ";
        std::cout << ((services[i].ServiceStatusProcess.dwCurrentState == SERVICE_RUNNING) ? "Running" : "Stopped") << "\n";
    }

    CloseServiceHandle(scm);
}

bool ServiceManager::startService(const std::string& name) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return false;

    SC_HANDLE svc = OpenServiceA(scm, name.c_str(), SERVICE_START);
    if (!svc) {
        CloseServiceHandle(scm);
        return false;
    }

    bool success = StartService(svc, 0, NULL);
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return success;
}

bool ServiceManager::stopService(const std::string& name) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return false;

    SC_HANDLE svc = OpenServiceA(scm, name.c_str(), SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (!svc) {
        CloseServiceHandle(scm);
        return false;
    }

    SERVICE_STATUS status;
    bool success = ControlService(svc, SERVICE_CONTROL_STOP, &status);
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return success;
}
