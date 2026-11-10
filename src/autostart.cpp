#include "autostart.h"
#include <windows.h>

static const wchar_t* RUN_KEY    = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t* VALUE_NAME = L"CallInClean";

bool isAutostartEnabled() {
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
        return false;
    LONG r = RegQueryValueExW(key, VALUE_NAME, nullptr, nullptr, nullptr, nullptr);
    RegCloseKey(key);
    return r == ERROR_SUCCESS;
}

bool setAutostart(bool enable) {
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, KEY_SET_VALUE, &key) != ERROR_SUCCESS)
        return false;

    LONG r;
    if (enable) {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(nullptr, path, MAX_PATH);

        wchar_t value[MAX_PATH + 3];          
        wsprintfW(value, L"\"%s\"", path);

        DWORD bytes = (lstrlenW(value) + 1) * sizeof(wchar_t);
        r = RegSetValueExW(key, VALUE_NAME, 0, REG_SZ, (const BYTE*)value, bytes);
    } else {
        r = RegDeleteValueW(key, VALUE_NAME);
        if (r == ERROR_FILE_NOT_FOUND) r = ERROR_SUCCESS;   
    }

    RegCloseKey(key);
    return r == ERROR_SUCCESS;
}