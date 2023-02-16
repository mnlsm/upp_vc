#include "Core.h"
//#BLITZ_APPROVE



#define DLLFILENAME "Kernel32.dll"
#define DLIMODULE   UnicodeWin32
#define DLIHEADER   <Core/Kernel32W.dli>
#include <Core/dli_source.h>

#define DLLFILENAME "Mpr.dll"
#define DLIMODULE   UnicodeWin32Net
#define DLIHEADER   <Core/Mpr32W.dli>
#include <Core/dli_source.h>

#define Ptr Ptr_
#define byte byte_
#define CY win32_CY_

#include <winnls.h>
#include <winnetwk.h>

#include <wincon.h>
#include <shlobj.h>

#undef Ptr
#undef byte
#undef CY



NAMESPACE_UPP

static int sDirSep(int c) {
    return c == '/' || c == '\\' ? c : 0;
}

static bool strecmp0(const char *p, const char *s) {
    while(*p) {
        if(*p == '*') {
            while(*p == '*') p++;
            do
                if(ToUpper(*p) == ToUpper(*s) && strecmp0(p, s)) return true;
            while(*s++);
            return false;
        }
        if(*p == '?') {
            if(*s == '\0') return false;
        }else if(ToUpper(*p) != ToUpper(*s)) return false;
        s++;
        p++;
    }
    return *s == '\0';
}

bool PatternMatch(const char *p, const char *s) {
    const char *q;
    q = strchr(p, '.');
    if(q) {
        if(q[1] == '\0') {
            if(strchr(s, '.')) return false;
            String h(p, q);
            return strecmp0(h, s);
        }else if(q[1] == '*' && q[2] == '\0') {
            String h(p, q);
            return strecmp0(h, s) || strecmp0(p, s);
        }
    }
    return strecmp0(p, s);
}

bool PatternMatchMulti(const char *p, const char *s) {
    String pt;
    while(*p) {
        if(*p == ';' || *p == ',' || *p == ' ') {
            if(PatternMatch(pt, s)) return true;
            p++;
            while(*p == ';' || *p == ',' || *p == ' ') p++;
            pt.Clear();
        }else
            pt.Cat(*p++);
    }
    return pt.IsEmpty() ? false : PatternMatch(pt, s);
}

const char *GetFileNamePos(const char *fileName) {
    const char *s = fileName;
    const char *fname = s;
    char c;
    while((c = *s++) != '\0')
        if(c == '\\' || c == ':' || c == '/')
            fname = s;
    return fname;
}

const char *GetFileExtPos(const char *fileName) {
    fileName = GetFileNamePos(fileName);
    const char *ext = strrchr(fileName, '.');
    return ext ? ext : fileName + strlen(fileName);
}

bool HasFileExt(const char *path) {
    return *GetFileExtPos(path);
}

bool HasWildcards(const char *fileName) {
    return strchr(fileName, '*') || strchr(fileName, '?');
}

bool IsFullPath(const char *r) {
    return *r && r[1] && (r[1] == ':' || r[0] == '\\' && r[1] == '\\' || r[0] == '/' && r[1] == '/');
}

String GetFileDirectory(const char *fileName) {
    return String(fileName, (int)(GetFileNamePos(fileName) - fileName));
}

String GetFileFolder(const char *fileName) {
    const char *s = GetFileNamePos(fileName);
    if(s - fileName == 3 && fileName[1] == ':')
        return String(fileName, 3);
    if(s > fileName)
        return String(fileName, (int)(s - fileName) - 1);
    return Null;
}

String GetFileTitle(const char *fileName) {
    fileName = GetFileNamePos(fileName);
    const char *ext = GetFileExtPos(fileName);
    if(*ext)
        return String(fileName, (int)(ext - fileName));
    else
        return fileName;
}

String GetFileExt(const char *fileName) {
    return GetFileExtPos(fileName);
}

String GetFileName(const char *fileName) {
    return GetFileNamePos(fileName);
}

String AppendFileName(const String& path, const char *fileName) {
    String result = path;
    if(result.GetLength() && *result.Last() != DIR_SEP && *fileName != DIR_SEP)
        result += DIR_SEP;
    result += fileName;
    return result;
}

bool   PathIsEqual(const char *p1, const char *p2) {
    return ToLower(NormalizePath(p1)) == ToLower(NormalizePath(p2));
}

String GetCurrentDirectoryA() {
    if(IsWinNT()) {
        wchar h[MAX_PATH];
        UnicodeWin32().GetCurrentDirectoryW(MAX_PATH, h);
        return FromSystemCharsetW(h);
    }else {
        char h[MAX_PATH];
        ::GetCurrentDirectoryA(MAX_PATH, h);
        return FromSystemCharset(h);
    }
}

String GetTempPathA() {
    if(IsWinNT()) {
        wchar h[MAX_PATH];
        UnicodeWin32().GetTempPathW(MAX_PATH, h);
        return FromSystemCharsetW(h);
    }else {
        char h[MAX_PATH];
        ::GetTempPathA(MAX_PATH, h);
        return FromSystemCharset(h);
    }
}

String GetTempFileName(const char *prefix) {
    Uuid id = Uuid::Create();
    return AppendFileName(GetTempPathA(), String(prefix) + Format(id) + ".tmp");
}

String FromUnixName(const char* fn, const char* stop = NULL) {
    String s;
    char c;
    while(fn != stop && (c = *fn++))
        s += (c == '/' ? '\\' : c);
    return s;
}

String ToUnixName(const char* fn, const char* stop = NULL) {
    String s;
    char c;
    while(fn != stop && (c = *fn++))
        s += (c == '\\' ? '/' : c);
    return s;
}

String GetFullPath(const char *file) {
    if(IsWinNT()) {
        String ufn = FromUnixName(file);
        wchar h[MAX_PATH];
        UnicodeWin32().GetFullPathNameW(ToSystemCharsetW(ufn), MAX_PATH, h, 0);
        return FromSystemCharsetW(h);
    }else {
        String ufn = FromUnixName(file);
        char h[MAX_PATH];
        GetFullPathNameA(ToSystemCharset(ufn), MAX_PATH, h, 0);
        return FromSystemCharset(h);
    }
}

String GetFileOnPath(const char* file, const char* paths, bool current, const char *curdir) {
    String ufn = NativePath(file);
    if(IsFullPath(ufn))
        return ufn;
    String fn;

    String cd = curdir;
    if(!curdir)
        cd = GetCurrentDirectoryA();
    if(current && FileExists(fn = NormalizePath(ufn, cd)))
        ;
    else if(paths) {
        fn = Null;
        while(*paths) {
            const char* start = paths;
            while(*paths && *paths != ';')
                paths++;

            String dir(start, (int)(paths - start));
            if(!dir.IsEmpty()) {
                dir = NormalizePath(AppendFileName(NativePath(dir), ufn), cd);
                if(FileExists(dir)) {
                    fn = dir;
                    break;
                }
            }
            if(*paths)
                paths++;
        }
    }
    return fn;
}

String WinPath(const char *p) {
    String r;
    while(*p) {
        r.Cat(*p == '/' ? '\\' : *p);
        p++;
    }
    return r;
}

String UnixPath(const char *p) {
    String r;
    while(*p) {
        r.Cat(*p == '\\' ? '/' : *p);
        p++;
    }
    return r;
}

String AppendExt(const char* fn, const char* ext) {
    String result = NativePath(fn);
    if(!HasFileExt(fn))
        result += ext;
    return result;
}

String ForceExt(const char* fn, const char* ext) {
    return NativePath(String(fn, GetFileExtPos(fn))) + ext;
}



void FindFile::Init() {
    if(IsWinNT()) {
        w = new WIN32_FIND_DATAW;
        a = NULL;
    }else {
        a = new WIN32_FIND_DATAA;
        w = NULL;
    }
}

FindFile::~FindFile() {
    Close();
    delete a;
    delete w;
}

FindFile::FindFile() {
    Init();
    handle = INVALID_HANDLE_VALUE;
}

FindFile::FindFile(const char *name) {
    Init();
    handle = INVALID_HANDLE_VALUE;
    Search(name);
}

bool FindFile::Search(const char *name) {
    pattern = GetFileName(name);
    path = GetFileDirectory(name);
    Close();
    if(w)
        handle = UnicodeWin32().FindFirstFileW(ToSystemCharsetW(name), w);
    else
        handle = FindFirstFileA(ToSystemCharset(name), a);
    if(handle == INVALID_HANDLE_VALUE)
        return false;
    if(!PatternMatch(pattern, GetName()))
        return Next();
    return true;
}

static bool sGetSymLinkPath0(const char *linkpath, String *path) {
    bool ret = false;
    HRESULT hres;
    IShellLinkA* psl;
    IPersistFile* ppf;
    CoInitialize(NULL);
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkA,
                            (PVOID *) &psl);
    if(SUCCEEDED(hres)) {
        hres = psl->QueryInterface(IID_IPersistFile, (PVOID *) &ppf);
        if(SUCCEEDED(hres)) {
            hres = ppf->Load(ToSystemCharsetW(linkpath), STGM_READ);
            if(SUCCEEDED(hres))
                if(path) {
                    char fileW[_MAX_PATH] = {0};
                    psl->GetPath(fileW, _MAX_PATH, NULL, 0);
                    *path = FromSystemCharset(fileW);
                }else
                    ret = true;
            ppf->Release();
        }
        psl->Release();
    }
    CoUninitialize();
    return ret;
}

bool FindFile::IsSymLink() const {
    String name = GetName();
    if(GetFileExt(name) != ".lnk")
        return false;
    return sGetSymLinkPath0(AppendFileName(path, name), NULL);
}

bool FindFile::IsExecutable() const {
    return !IsDirectory() && ToLower(GetName()).EndsWith(".exe");
}

void FindFile::Close() {
    if(handle != INVALID_HANDLE_VALUE) FindClose(handle);
    handle = INVALID_HANDLE_VALUE;
}

bool FindFile::Next0() {
    if(w) {
        if(!UnicodeWin32().FindNextFileW(handle, w)) {
            Close();
            return false;
        }
    }else {
        if(!FindNextFileA(handle, a)) {
            Close();
            return false;
        }
    }
    return true;
}

bool FindFile::Next() {
    for(;;) {
        if(!Next0())
            return false;
        if(PatternMatch(pattern, GetName()))
            return true;
    }
}

dword FindFile::GetAttributes() const {
    return w ? w->dwFileAttributes : a->dwFileAttributes;
}

String FindFile::GetName() const {
    return w ? FromSystemCharsetW(w->cFileName) : FromSystemCharset(a->cFileName);
}

int64 FindFile::GetLength() const {
    if(w)
        return (int64)w->nFileSizeLow | ((int64)w->nFileSizeHigh << 32);
    else
        return (int64)a->nFileSizeLow | ((int64)a->nFileSizeHigh << 32);
}

FileTime FindFile::GetCreationTime() const {
    return w ? w->ftCreationTime : a->ftCreationTime;
}

FileTime FindFile::GetLastAccessTime() const {
    return w ? w->ftLastAccessTime : a->ftLastAccessTime;
}

FileTime FindFile::GetLastWriteTime() const {
    return w ? w->ftLastWriteTime : a->ftLastWriteTime;
}

bool FindFile::IsDirectory() const {
    return w ? w->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
           : a->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}


bool FindFile::IsFolder() const {
    if(w)
        return IsDirectory()
               && !(w->cFileName[0] == '.' && w->cFileName[1] == 0)
               && !(w->cFileName[0] == '.' && w->cFileName[1] == '.' && w->cFileName[2] == 0);
    else
        return IsDirectory()
               && !(a->cFileName[0] == '.' && a->cFileName[1] == 0)
               && !(a->cFileName[0] == '.' && a->cFileName[1] == '.' && a->cFileName[2] == 0);
}

bool FindFile::IsArchive() const {
    return w ? w->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE
           : a->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE;
}

bool FindFile::IsCompressed() const {
    return w ? w->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED
           : a->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED;
}

bool FindFile::IsHidden() const {
    return w ? w->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN
           : a->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN;
}

bool FindFile::IsReadOnly() const {
    return w ? w->dwFileAttributes & FILE_ATTRIBUTE_READONLY
           : a->dwFileAttributes & FILE_ATTRIBUTE_READONLY;
}

bool FindFile::IsSystem() const {
    return w ? w->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM
           : a->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM;
}

bool FindFile::IsTemporary() const {
    return w ? w->dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY
           : a->dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY;
}

String NormalizePath(const char *path, const char *currdir) {
    String join_path;
    if(!IsFullPath(path))
        path = join_path = AppendFileName(currdir, path);
    String out;
    if(*path && path[1] == ':') {
        out << path[0] << ":\\";
        path += 3;
    }else if(path[0] == '\\' && path[1] == '\\') {
        out = "\\\\";
        path += 2;
    }else if(sDirSep(*path)) {
        if(*currdir)
            out << *currdir << ':';
        out.Cat(DIR_SEP);
        path++;
    }
    int outstart = out.GetLength();
    while(*path) {
        if(sDirSep(*path)) {
            while(sDirSep(*path))
                path++;
            if(*path == '\0')
                break;
            if(out.IsEmpty() || *out.Last() != DIR_SEP)
                out.Cat(DIR_SEP);
        }
        const char *b = path;
        while(*path && !sDirSep(*path))
            path++;
        if(path - b == 1 && *b == '.')
            ; //no-op
        else if(path - b == 2 && *b == '.' && b[1] == '.') {
            const char *ob = ~out + outstart, *oe = out.End();
            if(oe - 1 > ob && oe[-1] == DIR_SEP)
                oe--;
            while(oe > ob && oe[-1] != DIR_SEP)
                oe--;
            out.Trim((int)(oe - out.Begin()));
        }else
            out.Cat(b, (int)(path - b));
    }
    return out;
}




bool FileExists(const char *name) {
    FindFile ff(name);
    return ff && ff.IsFile();
}

int64 GetFileLength(const char *name) {
    FindFile ff(name);
    return ff ? ff.GetLength() : -1;
}

bool DirectoryExists(const char *name) {
    if(name[0] && name[1] == ':' && name[2] == '\\' && name[3] == 0 &&
            GetDriveTypeA(name) != DRIVE_NO_ROOT_DIR)
        return true;
    FindFile ff(name);
    return ff && ff.IsDirectory();
}

String NormalizePath(const char *path) {
    return NormalizePath(path, GetCurrentDirectoryA());
}

bool FileCopy(const char *oldname, const char *newname) {
    if(IsWinNT())
        return UnicodeWin32().CopyFileW(ToSystemCharsetW(oldname), ToSystemCharsetW(newname), false);
    else
        return CopyFileA(ToSystemCharset(oldname), ToSystemCharset(newname), false);
}

bool FileMove(const char *oldname, const char *newname) {
    if(IsWinNT())
        return !!UnicodeWin32().MoveFileW(ToSystemCharsetW(oldname), ToSystemCharsetW(newname));
    else
        return !!MoveFileA(ToSystemCharset(oldname), ToSystemCharset(newname));

}

bool FileDelete(const char *filename) {
    if(IsWinNT())
        return !!UnicodeWin32().DeleteFileW(ToSystemCharsetW(filename));
    else
        return !!DeleteFileA(ToSystemCharset(filename));
}

bool DirectoryDelete(const char *dirname) {
    if(IsWinNT())
        return !!UnicodeWin32().RemoveDirectoryW(ToSystemCharsetW(dirname));
    else
        return !!RemoveDirectoryA(ToSystemCharset(dirname));
}

int Compare_FileTime(const FileTime& fa, const FileTime& fb) {
    return CompareFileTime(&fa, &fb);
}

Time FileGetTime(const char *filename) {
    HANDLE handle;
    if(IsWinNT())
        handle = UnicodeWin32().CreateFileW(ToSystemCharsetW(filename), GENERIC_READ,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    else
        handle = CreateFileA(ToSystemCharset(filename), GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if(handle == INVALID_HANDLE_VALUE)
        return Null;
    FileTime ft;
    bool res = GetFileTime(handle, 0, 0, &ft);
    CloseHandle(handle);
    return res ? Time(ft) : Time(Null);
}

FileTime GetFileTime(const char *filename) {
    HANDLE handle;
    if(IsWinNT())
        handle = UnicodeWin32().CreateFileW(ToSystemCharsetW(filename), GENERIC_READ,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    else
        handle = CreateFileA(ToSystemCharset(filename), GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    FileTime ft0;
    memset(&ft0, 0, sizeof(ft0));
    if(handle == INVALID_HANDLE_VALUE)
        return ft0;
    FileTime ft;
    bool res = GetFileTime(handle, 0, 0, &ft);
    CloseHandle(handle);
    return res ? ft : ft0;
}

bool SetFileTime(const char *filename, FileTime ft) {
    HANDLE handle;
    if(IsWinNT())
        handle = UnicodeWin32().CreateFileW(ToSystemCharsetW(filename), GENERIC_READ | GENERIC_WRITE,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                            OPEN_EXISTING, 0, NULL);
    else
        handle = CreateFileA(ToSystemCharset(filename), GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                             OPEN_EXISTING, 0, NULL);
    if(handle == INVALID_HANDLE_VALUE)
        return false;
    bool res = SetFileTime(handle, 0, 0, &ft);
    CloseHandle(handle);
    return res;
}

bool FileSetTime(const char *filename, Time time) {
    return SetFileTime(filename, TimeToFileTime(time));
}

FileTime TimeToFileTime(Time time) {
    SYSTEMTIME tm;
    Zero(tm);
    tm.wYear   = time.year;
    tm.wMonth  = time.month;
    tm.wDay    = time.day;
    tm.wHour   = time.hour;
    tm.wMinute = time.minute;
    tm.wSecond = time.second;
    FileTime ftl, ftg;
    SystemTimeToFileTime(&tm, &ftl);
    LocalFileTimeToFileTime(&ftl, &ftg);
    return ftg;
}

bool DirectoryCreate(const char *path) {
    if(IsWinNT())
        return !!UnicodeWin32().CreateDirectoryW(ToSystemCharsetW(path), 0);
    else
        return !!CreateDirectoryA(ToSystemCharset(path), 0);
}

bool RealizePath(const String& file) {
    return RealizeDirectory(GetFileFolder(file));
}

#define DIR_MIN 3 //!! wrong! what about \a\b\c ?

bool RealizeDirectory(const String& d) {
    String dir = NormalizePath(d);
    Vector<String> p;
    while(dir.GetLength() > DIR_MIN) {
        p.Add(dir);
        dir = GetFileFolder(dir);
    }
    for(int i = p.GetCount() - 1; i >= 0; i--)
        if(!DirectoryExists(p[i]))
            if(!DirectoryCreate(p[i]))
                return false;
    return true;
}

bool DeleteFolderDeep(const char *dir) {
    {
        FindFile ff(AppendFileName(dir, "*.*"));
        while(ff) {
            String name = ff.GetName();
            String p = AppendFileName(dir, name);
            if(ff.IsFile())
                FileDelete(p);
            else if(ff.IsFolder())
                DeleteFolderDeep(p);
            ff.Next();
        }
    }
    return DirectoryDelete(dir);
}

String GetSymLinkPath(const char *linkpath) {
    String path;
    sGetSymLinkPath0(linkpath, &path);
    return path;
}

FileSystemInfo::FileInfo::FileInfo()
    : length(Null), read_only(false), is_directory(false)
    , is_folder(false), is_file(false), is_symlink(false), is_archive(false)
    , is_compressed(false), is_hidden(false), is_read_only(false), is_system(false)
    , is_temporary(false), root_style(ROOT_NO_ROOT_DIR)
{}

GLOBAL_VAR(FileSystemInfo, StdFileSystemInfo)

int FileSystemInfo::GetStyle() const {
    return STYLE_WIN32;
}

Array<FileSystemInfo::FileInfo> FileSystemInfo::Find(String mask, int max_count) const {
    Array<FileInfo> fi;
    if(IsNull(mask)) {
        char drive[4] = "?:\\";
        for(int c = 'A'; c <= 'Z'; c++) {
            *drive = c;
            int n = GetDriveTypeA(drive);
            if(n == DRIVE_NO_ROOT_DIR)
                continue;
            FileInfo& f = fi.Add();
            f.filename = drive;
            char name[256], system[256];
            DWORD d;
            if(c != 'A' && c != 'B' && n != DRIVE_REMOTE && n != DRIVE_UNKNOWN) {
                bool b = GetVolumeInformationA(drive, name, 256, &d, &d, &d, system, 256);
                if(b) {
                    if(*name) f.root_desc << " " << FromSystemCharset(name);
                }else if(n == DRIVE_REMOVABLE || n == DRIVE_CDROM) {
                    fi.Drop();
                    continue;
                }
            }
            switch(n) {
            default:
            case DRIVE_UNKNOWN:
                f.root_style = ROOT_UNKNOWN;
                break;
            case DRIVE_NO_ROOT_DIR:
                f.root_style = ROOT_NO_ROOT_DIR;
                break;
            case DRIVE_REMOVABLE:
                f.root_style = ROOT_REMOVABLE;
                break;
            case DRIVE_FIXED:
                f.root_style = ROOT_FIXED;
                break;
            case DRIVE_REMOTE:
                f.root_style = ROOT_REMOTE;
                break;
            case DRIVE_CDROM:
                f.root_style = ROOT_CDROM;
                break;
            case DRIVE_RAMDISK:
                f.root_style = ROOT_RAMDISK;
                break;
            }
        }
    }else {
        FindFile ff;
        if(ff.Search(mask))
            do {
                FileInfo& f = fi.Add();
                f.filename = ff.GetName();
                f.is_archive = ff.IsArchive();
                f.is_compressed = ff.IsCompressed();
                f.is_hidden = ff.IsHidden();
                f.is_system = ff.IsSystem();
                f.is_temporary = ff.IsTemporary();
                f.is_read_only = ff.IsReadOnly();
                f.length = ff.GetLength();
                f.last_access_time = ff.GetLastAccessTime();
                f.last_write_time = ff.GetLastWriteTime();
                f.creation_time = ff.GetCreationTime();
                f.unix_mode = 0;
                f.read_only = ff.IsReadOnly();
                f.is_directory = ff.IsDirectory();
                f.is_folder = ff.IsFolder();
                f.is_file = ff.IsFile();

            }while(ff.Next() && fi.GetCount() < max_count);
    }
    return fi;
}

bool FileSystemInfo::CreateFolder(String path, String& error) const {
    if(UPP::DirectoryCreate(path))
        return true;
    error = GetErrorMessage(GetLastError());
    return false;
}

bool FileSystemInfo::FolderExists(String path) const {
    if(IsNull(path))
        return true;
    if(path.Find('*') >= 0 || path.Find('?') >= 0)
        return false;
    Array<FileInfo> fi = Find(path, 1);
    return !fi.IsEmpty() && fi[0].is_directory;
}

END_UPP_NAMESPACE
