#include "hook.h"
#include "config.h"
#include <iostream>

using namespace std;

static HHOOK keyboardHook = nullptr;
static HHOOK mouseHook = nullptr;

static DWORD lockStart = 0;
static DWORD lockDuration = 0;
static bool kbLocked = false;
static bool msLocked = false;

static DWORD chordStart = 0;
static bool escDown = false;
static bool altDown = false;
static bool rightMouseDown = false;
static bool chordActive = false;
static bool unlockRequested = false;

static bool keyDown(int vk) {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

static bool gestureHeld() {
    if (kbLocked && msLocked) {
        return escDown && altDown && rightMouseDown;          
    }
    if (kbLocked) {
        return keyDown(VK_RBUTTON);    
    }
    if (msLocked) {
        return keyDown(VK_ESCAPE) && keyDown(VK_MENU);        
    }
    return false;
}

static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0 || isLockOver()) {
        return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
    }

    KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
    bool isDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
    bool isUp   = (wParam == WM_KEYUP   || wParam == WM_SYSKEYUP);

    if (kb->vkCode == VK_ESCAPE) {
        if (isDown) escDown = true;
        else if (isUp) escDown = false;
    } else if (kb->vkCode == VK_LMENU || kb->vkCode == VK_RMENU) {
        if (isDown) altDown = true;
        else if (isUp) altDown = false;
    }

    return 1;
}

static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0 || isLockOver()) {
        return CallNextHookEx(mouseHook, nCode, wParam, lParam);
    }

    if (wParam == WM_RBUTTONDOWN) rightMouseDown = true;
    else if (wParam == WM_RBUTTONUP) rightMouseDown = false;

    return 1;
}

bool lockKeyboard() {
    keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandleW(nullptr), 0);
    if (keyboardHook == nullptr) {
        cerr << "Error installing keyboard hook" << endl;
        return false;
    }
    return true;
}

bool lockMouse() {
    mouseHook = SetWindowsHookExW(WH_MOUSE_LL, MouseProc, GetModuleHandleW(nullptr), 0);
    if (mouseHook == nullptr) {
        cerr << "Error installing mouse hook" << endl;
        return false;
    }
    return true;
}

void unlockAll() {
    if (keyboardHook != nullptr) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = nullptr;
    }
    if (mouseHook != nullptr) {
        UnhookWindowsHookEx(mouseHook);
        mouseHook = nullptr;
    }
}

void startLock(DWORD durationMs, bool keyboardLocked, bool mouseLocked) {
    lockStart = GetTickCount();
    lockDuration = durationMs;
    kbLocked = keyboardLocked;
    msLocked = mouseLocked;
    escDown = altDown = rightMouseDown = false;
    chordActive = false;
    unlockRequested = false;
}

bool isLockOver() {
    return unlockRequested || (GetTickCount() - lockStart) >= lockDuration;
}

DWORD lockRemainingMs() {
    if (unlockRequested) return 0;
    DWORD elapsed = GetTickCount() - lockStart;
    return (elapsed >= lockDuration) ? 0 : (lockDuration - elapsed);
}

void checkChord() {
    bool held = gestureHeld();

    if (held && !chordActive) {
        chordActive = true;
        chordStart = GetTickCount();    
    } else if (!held) {
        chordActive = false;
    }

    if (chordActive && (GetTickCount() - chordStart) >= UNLOCK_HOLD_MS) {
        unlockRequested = true;
    }
}

bool unlockedByChord() {
    return unlockRequested;
}

const wchar_t* unlockHint() {
    if (kbLocked && msLocked) return L"Hold Esc + Alt + Right-click";
    if (kbLocked)             return L"Hold Right mouse button";
    return L"Hold Esc + Alt";
}