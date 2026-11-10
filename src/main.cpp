#include <windows.h>
#include <shellapi.h>
#include "hook.h"
#include "overlay.h"
#include "config.h"
#include "autostart.h"
#include "resource.h"

static const UINT WM_TRAY = WM_APP + 1;
static const UINT_PTR TIMER_ID = 1;

enum { CMD_KEYBOARD = 1001, CMD_MOUSE, CMD_BOTH, CMD_AUTOSTART, CMD_EXIT};

static HWND mainWnd = nullptr;
static bool locked = false;
static UINT wmTaskbarCreated = 0;
static NOTIFYICONDATAW nid = {};

static void addTrayIcon(HWND hwnd) {
    nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAY;
    nid.hIcon = (HICON)LoadImageW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDI_APPICON),
                                IMAGE_ICON,
                                GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                                LR_SHARED);
    if (nid.hIcon == nullptr) {
        nid.hIcon = LoadIconW(nullptr, MAKEINTRESOURCEW(32512)); 
    }
    lstrcpyW(nid.szTip, L"Call-In Clean");
    Shell_NotifyIconW(NIM_ADD, &nid);
}

static void removeTrayIcon() {
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

static void beginLock(bool kb, bool ms) {
    if (locked) return;

    startLock(LOCK_DURATION_MS, kb, ms);

    bool ok = true;
    if (kb && !lockKeyboard()) ok = false;
    if (ok && ms && !lockMouse()) ok = false;
    if (ok && !showOverlay()) ok = false;

    if (!ok) {
        unlockAll();
        hideOverlay();
        MessageBoxW(nullptr, L"Could not start the lock.", L"Call-In Clean",
                    MB_OK | MB_ICONERROR);
        return;
    }

    locked = true;
    SetTimer(mainWnd, TIMER_ID, 10, nullptr);
}

static void endLock() {
    if (!locked) return;
    KillTimer(mainWnd, TIMER_ID);
    unlockAll(); 
    hideOverlay();
    locked = false;
}

static void showTrayMenu(HWND hwnd) {
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, CMD_KEYBOARD, L"Lock keyboard");
    AppendMenuW(menu, MF_STRING, CMD_MOUSE,    L"Lock mouse");
    AppendMenuW(menu, MF_STRING, CMD_BOTH,     L"Lock keyboard + mouse");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING | (isAutostartEnabled() ? MF_CHECKED : 0), CMD_AUTOSTART, L"Start with Windows");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, CMD_EXIT,     L"Exit");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);   
    int cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_BOTTOMALIGN,
                             pt.x, pt.y, 0, hwnd, nullptr);
    PostMessageW(hwnd, WM_NULL, 0, 0);
    DestroyMenu(menu);

    switch (cmd) {
    case CMD_KEYBOARD: beginLock(true, false); break;
    case CMD_MOUSE:    beginLock(false, true); break;
    case CMD_BOTH:     beginLock(true, true);  break;
    case CMD_AUTOSTART: setAutostart(!isAutostartEnabled()); break;
    case CMD_EXIT:     endLock(); DestroyWindow(hwnd); break;
    }
}

static LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == wmTaskbarCreated) {   
        addTrayIcon(hwnd);
        return 0;
    }

    switch (msg) {
    case WM_TRAY:
        if (LOWORD(lParam) == WM_LBUTTONUP || LOWORD(lParam) == WM_RBUTTONUP) {
            showTrayMenu(hwnd);
        }
        return 0;

    case WM_TIMER:
        if (wParam == TIMER_ID) {
            checkChord();
            if (isLockOver()) endLock();
        }
        return 0;

    case WM_DESTROY:
        endLock();
        removeTrayIcon();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, int) {
    HANDLE mutex = CreateMutexW(nullptr, TRUE, L"CallInClean_SingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;

    wmTaskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = MainProc;
    wc.hInstance = inst;
    wc.lpszClassName = L"CallInCleanMain";
    if (!RegisterClassExW(&wc)) return 1;

    mainWnd = CreateWindowExW(0, L"CallInCleanMain", L"Call-In Clean", 0,
                              0, 0, 0, 0, nullptr, nullptr, inst, nullptr);
    if (mainWnd == nullptr) return 1;

    addTrayIcon(mainWnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    CloseHandle(mutex);
    return 0;
}