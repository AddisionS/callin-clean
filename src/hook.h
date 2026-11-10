#pragma once
#include <windows.h>

bool lockKeyboard();
bool lockMouse();
void unlockAll();

void startLock(DWORD durationMs, bool keyboardLocked, bool mouseLocked);
bool isLockOver();
DWORD lockRemainingMs();
void checkChord();
bool unlockedByChord();
const wchar_t* unlockHint();  