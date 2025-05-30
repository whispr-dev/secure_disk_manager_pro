#include <windows.h>
#include <shellapi.h>
#include <thread>
#include <atomic>
#include <chrono>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_STATUS 1002
#define ID_TRAY_CONTROL 1003

HINSTANCE hInst;
NOTIFYICONDATA nid;
std::atomic<bool> running{true};
std::thread monitorThread;

void MonitorLoop() {
    ProcWolfEngine engine("logs/procwolf.db", "whitelist/proc-wofl_whitelist.txt");
    while (running) {
        auto procs = engine.listProcesses(true);
        for (const auto& p : procs) {
            if (p.threatLevel >= 3 && !p.trusted) {
                engine.killProcess(p.pid, true);
                engine.logHistory(p, "Auto-killed by stealth tray");
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void ShowContextMenu(HWND hwnd) {
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_CONTROL, L"Open Control Panel");
    InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_STATUS, L"Check Status");
    InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_EXIT, L"Exit");

    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(hMenu);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_TRAYICON) {
        if (lParam == WM_RBUTTONUP) {
            ShowContextMenu(hwnd);
        }
    } else if (msg == WM_COMMAND) {
        switch (LOWORD(wParam)) {
            case ID_TRAY_EXIT:
                running = false;
                Shell_NotifyIcon(NIM_DELETE, &nid);
                PostQuitMessage(0);
                break;
            case ID_TRAY_STATUS:
                MessageBox(hwnd, running ? L"Proc-Wolf is monitoring." : L"Not monitoring.", L"Status", MB_OK);
                break;
            case ID_TRAY_CONTROL:
                ShellExecuteW(NULL, NULL, L"ProcWolfCLI.exe", L"list --assess", NULL, SW_SHOW);
                break;
        }
    } else if (msg == WM_DESTROY) {
        running = false;
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    hInst = hInstance;

    const wchar_t CLASS_NAME[] = L"ProcWolfTrayClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"ProcWolf", 0, 0, 0, 0, 0, 0, 0, hInst, 0);

    HICON hIcon = (HICON)LoadImageW(NULL, L"resources\\wolf.ico", IMAGE_ICON, 64, 64, LR_LOADFROMFILE);

    nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = hIcon;
    wcscpy_s(nid.szTip, L"Proc-Wolf Running...");
    Shell_NotifyIcon(NIM_ADD, &nid);

    monitorThread = std::thread(MonitorLoop);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    monitorThread.join();
    return 0;
}