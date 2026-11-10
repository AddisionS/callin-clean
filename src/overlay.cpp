#include "overlay.h"
#include "hook.h"
#include <windows.h>
#include "config.h"

static HWND overlayWnd = nullptr;
static const wchar_t* CLASS_NAME = L"CallInCleanOverlay";

static void drawCentered(HDC dc, const wchar_t* text, int fontHeight, COLORREF color, RECT area) {
    HFONT font = CreateFontW(-fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HGDIOBJ oldFont = SelectObject(dc, font);
    SetTextColor(dc, color);
    DrawTextW(dc, text, -1, &area, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, oldFont);
    DeleteObject(font);
}

static void paintOverlay(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC screenDc = BeginPaint(hwnd, &ps);

    RECT rc;
    GetClientRect(hwnd, &rc);

    HDC dc = CreateCompatibleDC(screenDc);
    HBITMAP bmp = CreateCompatibleBitmap(screenDc, rc.right, rc.bottom);
    HGDIOBJ oldBmp = SelectObject(dc, bmp);

    FillRect(dc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
    SetBkMode(dc, TRANSPARENT);

    DWORD secs = (lockRemainingMs() + 999) / 1000;  
    wchar_t timeText[32];
    wsprintfW(timeText, L"%u:%02u", (unsigned)(secs / 60), (unsigned)(secs % 60));

    int midY = rc.bottom / 2;
    RECT titleArea = { 0, midY - 220, rc.right, midY - 100 };
    RECT timeArea  = { 0, midY - 100, rc.right, midY + 100 };
    RECT hintArea  = { 0, midY + 120, rc.right, midY + 200 };

    drawCentered(dc, L"Call-In Clean", 48, RGB(160, 160, 160), titleArea);
    drawCentered(dc, timeText, 160, RGB(255, 255, 255), timeArea);
    wchar_t hint[128];
    wsprintfW(hint, L"%s for %u seconds to unlock", unlockHint(), (unsigned)(UNLOCK_HOLD_MS / 1000));
    drawCentered(dc, hint, 28, RGB(120, 120, 120), hintArea);

    BitBlt(screenDc, 0, 0, rc.right, rc.bottom, dc, 0, 0, SRCCOPY);

    SelectObject(dc, oldBmp);
    DeleteObject(bmp);
    DeleteDC(dc);
    EndPaint(hwnd, &ps);
}

static LRESULT CALLBACK OverlayProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, 1, 250, nullptr);  
        return 0;

    case WM_TIMER:
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;

    case WM_ERASEBKGND:
        return 1; 

    case WM_PAINT:
        paintOverlay(hwnd);
        return 0;

    case WM_CLOSE:
        return 0;  

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool showOverlay() {
    HINSTANCE inst = GetModuleHandleW(nullptr);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = OverlayProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassExW(&wc)) return false;

    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    overlayWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME, L"Call-In Clean", WS_POPUP,
        x, y, w, h, nullptr, nullptr, inst, nullptr);

    if (overlayWnd == nullptr) {
        UnregisterClassW(CLASS_NAME, inst);
        return false;
    }

    ShowWindow(overlayWnd, SW_SHOW);
    UpdateWindow(overlayWnd);
    return true;
}

void hideOverlay() {
    if (overlayWnd != nullptr) {
        DestroyWindow(overlayWnd);
        overlayWnd = nullptr;
    }
    UnregisterClassW(CLASS_NAME, GetModuleHandleW(nullptr));
}