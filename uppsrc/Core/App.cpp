#include "Core.h"

#define Ptr Ptr_
#define byte byte_
#define CY win32_CY_

#include <shellapi.h>
#include <wincon.h>
#include <shlobj.h>

#undef Ptr
#undef byte
#undef CY

NAMESPACE_UPP



String GetEnv(const char *id) {
    return WString(_wgetenv(WString(id))).ToString();
}

String GetExeFilePath() {
    return GetModuleFileNameA();
}

String  GetHomeDirectory() {
    return GetEnv("HOMEDRIVE") + GetEnv("HOMEPATH");
}



String GetExeDirFile(const char *filename) {
    return AppendFileName(GetFileFolder(GetExeFilePath()), filename);
}

String GetExeTitle() {
    return GetFileTitle(GetExeFilePath());
}


String  GetHomeDirFile(const char *fp) {
    return AppendFileName(GetHomeDirectory(), fp);
}

static bool sHomecfg;

void    UseHomeDirectoryConfig(bool b) {
    sHomecfg = b;
}

String  ConfigFile(const char *file) {
    if(sHomecfg) {
        String p = GetHomeDirFile(GetExeTitle());
        ONCELOCK
        RealizeDirectory(p);
        return AppendFileName(p, file);
    }
    return GetExeDirFile(file);
}

String  ConfigFile() {
    return ConfigFile(GetExeTitle() + ".cfg");
}

GLOBAL_VAR(Vector<WString>, coreCmdLine__)

const Vector<String>& CommandLine() {
    Vector<String> *ptr;
    INTERLOCKED {
        static ArrayMap< byte, Vector<String> > charset_cmd;
        byte cs = GetDefaultCharset();
        int f = charset_cmd.Find(cs);
        if(f >= 0)
            ptr = &charset_cmd[f];
        else {
            ptr = &charset_cmd.Add(cs);
            const Vector<WString>& src = coreCmdLine__();
            for(int i = 0; i < src.GetCount(); i++)
                ptr->Add(src[i].ToString());
        }
    }
    return *ptr;
}

VectorMap<WString, WString>& EnvMap() {
    static VectorMap<WString, WString> x;
    return x;
}

const VectorMap<String, String>& Environment() {
    VectorMap<String, String> *ptr;
    INTERLOCKED {
        static ArrayMap< byte, VectorMap<String, String> > charset_env;
        byte cs = GetDefaultCharset();
        int f = charset_env.Find(cs);
        if(f >= 0)
            ptr = &charset_env[f];
        else {
            ptr = &charset_env.Add(cs);
            const VectorMap<WString, WString>& env_map = EnvMap();
            for(int i = 0; i < env_map.GetCount(); i++)
                ptr->Add(env_map.GetKey(i).ToString(), env_map[i].ToString());
        }
    }
    return *ptr;
}

static int exitcode;
static bool sMainRunning;

void  SetExitCode(int code) {
    exitcode = code;
}
int   GetExitCode() {
    return exitcode;
}

bool  IsMainRunning() {
    return sMainRunning;
}

void LoadLangFiles(const char *dir) {
    FindFile ff(AppendFileName(dir, "*.tr"));
    while(ff) {
        LoadLngFile(AppendFileName(dir, ff.GetName()));
        ff.Next();
    }
}

void CommonInit() {
    LoadLangFiles(GetFileFolder(GetExeFilePath()));

    Vector<WString>& cmd = coreCmdLine__();
    static WString exp_cmd = "--export-tr";
    static WString brk_cmd = "--memory-breakpoint__";

    for(int i = 0; i < cmd.GetCount();) {
        if(cmd[i] == exp_cmd) {
            if(TRUE) {
                i++;
                int lang = 0;
                byte charset = CHARSET_UTF8;
                String fn = "all";
                if(i < cmd.GetCount()) {
                    if(cmd[i].GetLength() != 4 && cmd[i].GetLength() != 5) {
                        lang = 0;
                    }else {
                        lang = LNGFromText(cmd[i].ToString());
                        fn = cmd[i].ToString();
                        int c = cmd[i][4];
                        if(c >= '0' && c <= '8')
                            charset = c - '0' + CHARSET_WIN1250;
                        if(c >= 'A' && c <= 'J')
                            charset = c - 'A' + CHARSET_ISO8859_1;
                    }
                }
                fn << ".tr";
                FileOut out(GetExeDirFile(fn));
                if(lang)
                    SaveLngFile(out, SetLNGCharset(lang, charset));
                else {
                    Index<int> l = GetLngSet();
                    for(int i = 0; i < l.GetCount(); i++)
                        SaveLngFile(out, SetLNGCharset(l[i], charset));
                }
            }
            exit(0);
        }
#if defined(_DEBUG) && defined(UPP_HEAP)
        if(cmd[i] == brk_cmd && i + 1 < cmd.GetCount()) {
            MemoryBreakpoint(atoi(cmd[i + 1].ToString()));
            cmd.Remove(i, 2);
        }else
            i++;
#else
        i++;
#endif
    }
    sMainRunning = true;
}


void AppInitEnvironment__() {
    wchar *env = GetEnvironmentStringsW();
    for(wchar *ptr = env; *ptr; ptr++) {
        const wchar *b = ptr;
        if(*ptr)
            ptr++;
        while(*ptr && *ptr != '=')
            ptr++;
        WString varname(b, ptr);
        if(*ptr)
            ptr++;
        b = ptr;
        while(*ptr)
            ptr++;
        EnvMap().GetAdd(ToUpper(varname)) = WString(b, ptr);
    }
    FreeEnvironmentStringsW(env);
    CommonInit();
}

void AppInit__(int argc, const char **argv) {
    SetLanguage(LNG_ENGLISH);
    Vector<WString>& cmd = coreCmdLine__();
    for(int i = 1; i < argc; i++)
        cmd.Add(argv[i]);
    AppInitEnvironment__();
}

void AppExit__() {
    sMainRunning = false;

}

void    LaunchWebBrowser(const String& url) {
    ShellExecuteA(NULL, "open", url, NULL, ".", SW_SHOWDEFAULT);
}

String GetDataFile(const char *filename) {
    String s = GetEnv("UPP_MAIN__");
    return s.GetCount() ? AppendFileName(s, filename) : GetExeDirFile(filename);
}

String GetComputerName() {
    char temp[256];
    *temp = 0;
    dword w = 255;
    ::GetComputerNameA(temp, &w);
    return FromSystemCharset(temp);
}

String GetUserName() {
    char temp[256];
    *temp = 0;
    dword w = 255;
    ::GetUserNameA(temp, &w);
    return FromSystemCharset(temp);
}

String GetDesktopManager() {
    return "windows";
}

String GetShellFolder(int clsid) {
    wchar path[MAX_PATH];
    if(SHGetFolderPathW(NULL, clsid, NULL, /*SHGFP_TYPE_CURRENT*/0, path) == S_OK)
        return FromUnicodeBuffer(path);
    return Null;
}

String GetDesktopFolder() {
    return GetShellFolder(CSIDL_DESKTOP);
}
String GetProgramsFolder() {
    return GetShellFolder(CSIDL_PROGRAM_FILES);
}
String GetAppDataFolder() {
    return GetShellFolder(CSIDL_APPDATA);
}
String GetMusicFolder() {
    return GetShellFolder(CSIDL_MYMUSIC);
}
String GetPicturesFolder() {
    return GetShellFolder(CSIDL_MYPICTURES);
}
String GetVideoFolder() {
    return GetShellFolder(CSIDL_MYVIDEO);
}
String GetDocumentsFolder() {
    return GetShellFolder(/*CSIDL_MYDOCUMENTS*/0x0005);
}
String GetTemplatesFolder() {
    return GetShellFolder(CSIDL_TEMPLATES);
}

#define MY_DEFINE_KNOWN_FOLDER(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
static const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

MY_DEFINE_KNOWN_FOLDER(MY_FOLDERID_Downloads, 0x374de290, 0x123f, 0x4565, 0x91, 0x64, 0x39, 0xc4, 0x92, 0x5e, 0x46, 0x7b);

String GetDownloadFolder() {
    static HRESULT(STDAPICALLTYPE * SHGetKnownFolderPath)(const void * rfid, DWORD dwFlags, HANDLE hToken, PWSTR * ppszPath);
    ONCELOCK {
        DllFn(SHGetKnownFolderPath, "shell32.dll", "SHGetKnownFolderPath");
    }
    if(SHGetKnownFolderPath) {
        PWSTR path = NULL;
        if(SHGetKnownFolderPath(&MY_FOLDERID_Downloads, 0, NULL, &path) == S_OK && path) {
            String s = FromUnicodeBuffer(path, wstrlen(path));
            CoTaskMemFree(path);
            return s;
        }
    }
    return Null;
};



END_UPP_NAMESPACE
