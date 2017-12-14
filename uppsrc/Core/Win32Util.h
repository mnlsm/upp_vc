
#include <winreg.h>


inline bool IsWinNT() {
    return GetVersion() < 0x80000000;
}
bool IsWin2K();
bool IsWinXP();
bool IsWinVista();

HINSTANCE AppGetHandle();
void      AppSetHandle(HINSTANCE dll_instance);

String AsString(const wchar_t *buffer);
String AsString(const wchar_t *buffer, int count);
String AsString(const wchar_t *buffer, const wchar_t *end);

String GetWinRegString(const char *value, const char *path, HKEY base_key = HKEY_LOCAL_MACHINE);
int    GetWinRegInt(const char *value, const char *path, HKEY base_key = HKEY_LOCAL_MACHINE);
bool   SetWinRegString(const String& string, const char *value, const char *path, HKEY base_key = HKEY_LOCAL_MACHINE);
bool   SetWinRegExpandString(const String& string, const char *value, const char *path, HKEY base_key);
bool   SetWinRegInt(int data, const char *value, const char *path, HKEY base_key = HKEY_LOCAL_MACHINE);
void   DeleteWinReg(const String& key, HKEY base = HKEY_LOCAL_MACHINE);

void  *GetDllFn(const char *dll, const char *fn);

template <class T>
void   DllFn(T& x, const char *dll, const char *fn) {
    x = (T)GetDllFn(dll, fn);
}


String GetSystemDirectory();
String GetWindowsDirectory();

String GetModuleFileNameA(HINSTANCE instance = AppGetHandle());

//deprecated
class SyncObject {
protected:
    HANDLE     handle;

public:
    bool       Wait(int time_ms);
    bool       Wait();

    HANDLE     GetHandle() const {
        return handle;
    }

    SyncObject();
    ~SyncObject();
};

//deprecated
class Event : public SyncObject {
public:
    void       Set();

    Event();
};

