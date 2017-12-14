
#pragma comment( lib , "Mpr.lib" )
bool PatternMatch(const char *p, const char *s);
bool PatternMatchMulti(const char *p, const char *s);

const char  *GetFileNamePos(const char *path);
const char  *GetFileExtPos(const char *path);

bool    HasFileExt(const char *path);
bool    HasWildcards(const char *path);
bool    IsFullPath(const char *path);

String  GetFileDirectory(const char *path);   // with DIR_SEP at the end
String  GetFileFolder(const char *path);   // without DIR_SEP at the end, if not Win32 root
String  GetFileTitle(const char *path);
String  GetFileExt(const char *path);
String  GetFileName(const char *path);

String  AppendFileName(const String& path, const char *filename);

String WinPath(const char *path);
String UnixPath(const char *path);

inline String  NativePath(const char *path) {
    return WinPath(path);
}



String  AppendExt(const char *path, const char *ext);
String  ForceExt(const char *path, const char *ext);

String  GetFileOnPath(const char *file, const char *paths, bool current = true, const char *curdir = NULL);

String  GetFullPath(const char *path);
String  GetCurrentDirectoryA();

struct FileTime;

int Compare_FileTime(const FileTime& fa, const FileTime& fb);


struct FileTime : FILETIME, CompareRelOps<const FileTime&, &Compare_FileTime> {
    FileTime()                          {}
    FileTime(const FILETIME& ft) {
        dwLowDateTime = ft.dwLowDateTime;
        dwHighDateTime = ft.dwHighDateTime;
    }
};

class  FindFile {
    WIN32_FIND_DATAA  *a;
    WIN32_FIND_DATAW *w;
    HANDLE            handle;
    String            pattern;
    String            path;

    void        Init();
    bool        Next0();
    void        Close();

public:
    bool        Search(const char *path);
    bool        Next();

    dword       GetAttributes() const;
    String      GetName() const;
    int64       GetLength() const;
    FileTime    GetCreationTime() const;
    FileTime    GetLastAccessTime() const;
    FileTime    GetLastWriteTime() const;

    bool        IsDirectory() const;
    bool        IsFolder() const;
    bool        IsFile() const {
        return !IsDirectory();
    }
    bool        IsSymLink() const;
    bool        IsExecutable() const;

    bool        IsArchive() const;
    bool        IsCompressed() const;
    bool        IsHidden() const;
    bool        IsReadOnly() const;
    bool        IsSystem() const;
    bool        IsTemporary() const;

    operator    bool() const {
        return handle != INVALID_HANDLE_VALUE;
    }

    FindFile();
    FindFile(const char *name);
    ~FindFile();
};




int64       GetFileLength(const char *path);
bool        FileExists(const char *path);
bool        DirectoryExists(const char *path);

struct Time;
FileTime    GetFileTime(const char *path);
Time        FileGetTime(const char *path);
bool        SetFileTime(const char *path, FileTime ft);
bool        FileSetTime(const char *path, Time time);
FileTime    TimeToFileTime(Time time);

bool        FileCopy(const char *oldpath, const char *newpath);
bool        FileMove(const char *oldpath, const char *newpath);
bool        FileDelete(const char *path);


bool        DirectoryCreate(const char *path);
bool        RealizeDirectory(const String& path);
bool        RealizePath(const String& path);

bool        DirectoryDelete(const char *path);

String      NormalizePath(const char *path, const char *currdir);
String      NormalizePath(const char *path);

bool        PathIsEqual(const char *p1, const char *p2);



bool    DeleteFolderDeep(const char *dir);

String  GetTempPathA();
String  GetTempFileName(const char *prefix = NULL);

String GetSymLinkPath(const char *linkpath);

template <class T>
class Array;

class FileSystemInfo {
public:
    enum {
        ROOT_UNKNOWN     = 0,
        ROOT_NO_ROOT_DIR = 1,
        ROOT_REMOVABLE   = 2,
        ROOT_FIXED       = 3,
        ROOT_REMOTE      = 4,
        ROOT_CDROM       = 5,
        ROOT_RAMDISK     = 6,
        ROOT_NETWORK     = 7,
        ROOT_COMPUTER    = 8,
    };

    enum {
        STYLE_WIN32      = 0x0001,
        STYLE_POSIX      = 0x0002,
    };

    struct FileInfo {
        FileInfo();

        operator bool () const {
            return !IsNull(filename);
        }

        String filename;
        String msdos_name;
        String root_desc;
        int64  length;
        Time   last_access_time;
        Time   last_write_time;
        Time   creation_time;
        bool   read_only;
        bool   is_directory;
        bool   is_folder;
        bool   is_file;
        bool   is_symlink;
        bool   is_archive;
        bool   is_compressed;
        bool   is_hidden;
        bool   is_read_only;
        bool   is_system;
        bool   is_temporary;
        char   root_style;
        dword  unix_mode;
    };

    virtual int             GetStyle() const;
    bool                    IsWin32() const {
        return GetStyle() & STYLE_WIN32;
    }
    bool                    IsPosix() const {
        return GetStyle() & STYLE_POSIX;
    }

    virtual Array<FileInfo> Find(String mask, int max_count = 1000000) const;   // mask = Null -> root
    virtual bool            CreateFolder(String path, String& error) const;

    bool                    FolderExists(String path) const;

    virtual ~FileSystemInfo() {}
};

FileSystemInfo& StdFileSystemInfo();

class NetNode : Moveable<NetNode> {
    NETRESOURCEA net;
    String      local, remote, comment, provider;

    String      name;
    String      path;

    static void           Copy(String& t, char *s);
    static Array<NetNode> Enum0(HANDLE hEnum);
    static void           SetPtr(String& s, char *& ptr);

    void SetPtrs();

public:
    enum {
        UNKNOWN, NETWORK, GROUP, SERVER, SHARE
    };
    String         GetName() const {
        return name;
    }
    String         GetPath() const {
        return path;
    }
    int            GetDisplayType() const;
    Array<NetNode> Enum() const;

    void           Serialize(Stream& s);

    static Array<NetNode> EnumRoot();
    static Array<NetNode> EnumRemembered();

    NetNode();
    NetNode(const NetNode& s) {
        *this = s;
    }

    NetNode& operator=(const NetNode& s);
};
