#include <fstream>
#include <vector>
#include <string>

#define ID_LISTBOX 101
#define ID_EDITBOX 102
#define ID_ADD_BTN 103
#define ID_REMOVE_BTN 104
#define ID_SAVE_BTN 105

HWND hListBox, hEditBox;
std::vector<std::wstring> whitelist;

void LoadWhitelist() {
    whitelist.clear();
    std::wifstream infile(L"whitelist\\proc-wofl_whitelist.txt");
    std::wstring line;
    while (std::getline(infile, line)) {
        if (!line.empty() && line[0] != L'#') {
            whitelist.push_back(line);
        }
    }
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    for (const auto& entry : whitelist) {
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)entry.c_str());
    }
}

void SaveWhitelist() {
    std::wofstream outfile(L"whitelist\\proc-wofl_whitelist.txt");
    outfile << L"# proc-wolf whitelist\n";
    for (const auto& line : whitelist) {
        outfile << line << L"\n";
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        hListBox = CreateWindowW(L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_STANDARD,
            20, 20, 340, 200, hwnd, (HMENU)ID_LISTBOX, NULL, NULL);

        hEditBox = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
            20, 230, 240, 25, hwnd, (HMENU)ID_EDITBOX, NULL, NULL);

        CreateWindowW(L"BUTTON", L"Add", WS_CHILD | WS_VISIBLE,
            270, 230, 90, 25, hwnd, (HMENU)ID_ADD_BTN, NULL, NULL);

        CreateWindowW(L"BUTTON", L"Remove", WS_CHILD | WS_VISIBLE,
            20, 270, 90, 25, hwnd, (HMENU)ID_REMOVE_BTN, NULL, NULL);

        CreateWindowW(L"BUTTON", L"Save", WS_CHILD | WS_VISIBLE,
            270, 270, 90, 25, hwnd, (HMENU)ID_SAVE_BTN, NULL, NULL);

        LoadWhitelist();
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_ADD_BTN: {
            wchar_t buffer[256];
            GetWindowTextW(hEditBox, buffer, 256);
            if (wcslen(buffer) > 0) {
                std::wstring entry = buffer;
                whitelist.push_back(entry);
                SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)entry.c_str());
                SetWindowTextW(hEditBox, L"");
            }
            break;
        }
        case ID_REMOVE_BTN: {
            int sel = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
            if (sel != LB_ERR) {
                SendMessage(hListBox, LB_DELETESTRING, sel, 0);
                whitelist.erase(whitelist.begin() + sel);
            }
            break;
        }
        case ID_SAVE_BTN:
            SaveWhitelist();
            MessageBoxW(hwnd, L"Whitelist saved successfully!", L"Success", MB_OK | MB_ICONINFORMATION);
            break;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"WhitelistWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Proc-Wolf Whitelist Manager",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 350,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}