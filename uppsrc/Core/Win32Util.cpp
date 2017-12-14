#include "Core.h"

NAMESPACE_UPP



static HINSTANCE app_instance;

HINSTANCE AppGetHandle() {
    if(!app_instance)
        app_instance = GetModuleHandle(NULL);
    return app_instance;
}

void AppSetHandle(HINSTANCE dll_instance) {
    app_instance = dll_instance;
}

bool IsWin2K() {
    OSVERSIONINFO of;
    of.dwOSVersionInfoSize = sizeof(of);
    GetVersionEx(&of);
    return of.dwMajorVersion >= 5;
}

bool IsWinXP() {
    OSVERSIONINFO of;
    of.dwOSVersionInfoSize = sizeof(of);
    GetVersionEx(&of);
    return of.dwMajorVersion > 5 || of.dwMajorVersion == 5 && of.dwMinorVersion >= 1;
}

bool IsWinVista() {
    OSVERSIONINFO of;
    of.dwOSVersionInfoSize = sizeof(of);
    GetVersionEx(&of);
    return of.dwMajorVersion >= 6;
}

String AsString(const wchar_t *buffer) {
    if(!buffer)
        return Null;
    return AsString(buffer, (int)wcslen(buffer));
}

String AsString(const wchar_t *buffer, int count) { // Convert with code page...
    if(!buffer)
        return Null;
    StringBuffer temp(count);
    for(char *p = temp, *e = p + count; p < e;)
        *p++ = (char) * buffer++;
    return temp;
}

String AsString(const wchar_t *buffer, const wchar_t *end) {
    if(!buffer)
        return Null;
    return AsString(buffer, (int)(end - buffer));
}

String GetWinRegString(const char *value, const char *path, HKEY base_key) {
    HKEY key = 0;
    if(RegOpenKeyExA(base_key, path, 0, KEY_READ, &key) != ERROR_SUCCESS)
        return String::GetVoid();
    dword type, data;
    if(RegQueryValueExA(key, value, 0, &type, NULL, &data) != ERROR_SUCCESS) {
        RegCloseKey(key);
        return String::GetVoid();
    }
    StringBuffer raw_data(data);
    if(RegQueryValueExA(key, value, 0, 0, (byte *)~raw_data, &data) != ERROR_SUCCESS) {
        RegCloseKey(key);
        return String::GetVoid();
    }
    if(data > 0 && (type == REG_SZ || type == REG_EXPAND_SZ))
        data--;
    raw_data.SetLength(data);
    RegCloseKey(key);
    return raw_data;
}

int GetWinRegInt(const char *value, const char *path, HKEY base_key) {
    HKEY key = 0;
    if(RegOpenKeyExA(base_key, path, 0, KEY_READ, &key) != ERROR_SUCCESS)
        return Null;
    int data;
    dword type, length = sizeof(data);
    if(RegQueryValueExA(key, value, 0, &type, (byte *)&data, &length) != ERROR_SUCCESS)
        data = Null;
    RegCloseKey(key);
    return data;
}

bool SetWinRegString(const String& string, const char *value, const char *path, HKEY base_key) {
    HKEY key = 0;
    if(RegCreateKeyExA(base_key, path, 0, NULL, REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS, NULL, &key, NULL) != ERROR_SUCCESS)
        return false;
    bool ok = (RegSetValueExA(key, value, 0,    REG_SZ, string, string.GetLength() + 1) == ERROR_SUCCESS);
    RegCloseKey(key);
    return ok;
}

bool SetWinRegExpandString(const String& string, const char *value, const char *path, HKEY base_key) {
    HKEY key = 0;
    if(RegCreateKeyExA(base_key, path, 0, NULL, REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS, NULL, &key, NULL) != ERROR_SUCCESS)
        return false;
    bool ok = (RegSetValueExA(key, value, 0,    REG_EXPAND_SZ, string, string.GetLength() + 1) == ERROR_SUCCESS);
    RegCloseKey(key);
    return ok;
}

bool SetWinRegInt(int data, const char *value, const char *path, HKEY base_key) {
    HKEY key = 0;
    if(RegCreateKeyExA(base_key, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL) != ERROR_SUCCESS)
        return false;
    bool ok = (RegSetValueExA(key, value, 0, REG_DWORD, (const byte *)&data, sizeof(data)) == ERROR_SUCCESS);
    RegCloseKey(key);
    return ok;
}

void DeleteWinReg(const String& key, HKEY base) {
    HKEY hkey;
    if(RegOpenKeyExA(base, key, 0, KEY_READ, &hkey) != ERROR_SUCCESS)
        return;
    Vector<String> subkeys;
    char temp[_MAX_PATH];
    dword len;
    for(dword dw = 0; len = sizeof(temp), RegEnumKeyExA(hkey, dw, temp, &len, 0, 0, 0, 0) == ERROR_SUCCESS; dw++)
        subkeys.Add(temp);
    RegCloseKey(hkey);
    while(!subkeys.IsEmpty())
        DeleteWinReg(key + '\\' + subkeys.Pop(), base);
    RegDeleteKeyA(base, key);
}

String GetSystemDirectory() {
    char temp[MAX_PATH];
    *temp = 0;
    ::GetSystemDirectoryA(temp, sizeof(temp));
    return FromSystemCharset(temp);
}

String GetWindowsDirectory() {
    if(IsWinNT()) {
        wchar temp[MAX_PATH];
        *temp = 0;
        UnicodeWin32().GetWindowsDirectoryW(temp, sizeof(temp));
        return FromSystemCharsetW(temp);
    }else {
        char temp[MAX_PATH];
        *temp = 0;
        ::GetWindowsDirectoryA(temp, sizeof(temp));
        return FromSystemCharset(temp);
    }
}

void *GetDllFn(const char *dll, const char *fn) {
    if(HMODULE hDLL = LoadLibraryA(dll))
        return (void *)GetProcAddress(hDLL, fn);
    return NULL;
}

String GetModuleFileNameA(HINSTANCE instance) {
    if(IsWinNT()) {
        wchar h[_MAX_PATH];
        UnicodeWin32().GetModuleFileNameW(instance, h, _MAX_PATH);
        return FromSystemCharsetW(h);
    }else {
        char h[_MAX_PATH];
        GetModuleFileNameA(instance, h, _MAX_PATH);
        return FromSystemCharset(h);
    }
}

bool SyncObject::Wait(int ms) {
    return WaitForSingleObject(handle, ms);
}

bool SyncObject::Wait() {
    return Wait(INFINITE);
}

SyncObject::SyncObject() {
    handle = NULL;
}

SyncObject::~SyncObject() {
    if(handle) CloseHandle(handle);
}

Event::Event() {
    handle = CreateEvent(NULL, FALSE, FALSE, NULL);
}

void Event::Set() {
    SetEvent(handle);
}


END_UPP_NAMESPACE
