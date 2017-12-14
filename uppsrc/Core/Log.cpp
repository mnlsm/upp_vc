#include "Core.h"

NAMESPACE_UPP

LogStream::LogStream() {
    hfile = INVALID_HANDLE_VALUE;
    part = 0;
    sizelimit = 0;
    *filename = 0;
    options = LOG_FILE;
    depth = 0;
    bol = false;
}

LogStream::~LogStream() {}

void LogStream::Close() {
    if(hfile != INVALID_HANDLE_VALUE)
        CloseHandle(hfile);
    hfile = INVALID_HANDLE_VALUE;
}

bool LogStream::Delete() {
    Close();
    if(*filename) {
        if(!FileDelete(filename)) {
            BugLog() << "Error deleting " << filename << ": " << GetLastErrorMessage();
            return false;
        }
        *filename = 0;
    }
    return true;
}

void LogStream::Create(const char *path, bool append) {
    Close();

    strcpy(filename, path);
    strcpy(backup, filename);
    strcat(backup, ".old");

    DeleteFileA(backup);

    MoveFileA(filename, backup);


    filesize = 0;

    hfile = CreateFileA(ToSysChrSet(filename),
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        append ? OPEN_ALWAYS : CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                       );
    if(append)
        filesize = (int)SetFilePointer(hfile, 0, NULL, FILE_END);

    wrlim = ptr = (byte *)this;
    p = buffer;

    Time t = GetSysTime();

    char exe[512];
    char user[500];
    *user = 0;

    GetModuleFileNameA(AppGetHandle(), exe, 512);
    dword w = 2048;
    ::GetUserNameA(user, &w);


    char h[1000];
    sprintf(h, "* %s %02d.%02d.%04d %02d:%02d:%02d, user: %s\n",
            FromSysChrSet(exe),
            t.day, t.month, t.year, t.hour, t.minute, t.second, user);
    dword n;
    WriteFile(hfile, h, (dword)strlen(h), &n, NULL);
    if(part) {
        sprintf(h, ", #%d", part);
        WriteFile(hfile, h, (dword)strlen(h) , &n, NULL);
    }
    WriteFile(hfile, "\r\n", 2, &n, NULL);
    bol = true;
}

void LogStream::Flush() {
    int count = (int)(p - buffer);
    if(count == 0) return;
    if(options & LOG_COUT)
        Cout().Put(buffer, count);
    if(options & LOG_CERR)
        Cerr().Put(buffer, count);
    if(options & LOG_FILE)
        if(hfile != INVALID_HANDLE_VALUE) {
            dword n;
            WriteFile(hfile, buffer, count, &n, NULL);
        }
    if(options & LOG_DBG) {
        *p = 0;
        ::OutputDebugStringA((LPCSTR)buffer);
    }

    filesize += count;
    p = buffer;
    if(sizelimit > 0 && filesize > sizelimit)
        Create(filename, false);
}

void LogStream::Put0(int w) {
    if(w == LOG_BEGIN)
        depth = min(depth + 1, 20);
    else if(w == LOG_END)
        depth = max(depth - 1, 0);
    else {
        if(bol) {
            bol = false;
            for(int q = depth; q--;)
                Put0('\t');
            if(options & LOG_TIMESTAMP) {
                char h[60];
                Time t = GetSysTime();
                sprintf(h, "%02d.%02d.%04d %02d:%02d:%02d ",
                        t.day, t.month, t.year, t.hour, t.minute, t.second);
                const char *s = h;
                while(*s)
                    Put0(*s++);
            }
        }
        *p++ = w;
        if(w == '\n') {
            Flush();
            bol = true;
        }else if(p == buffer + 512)
            Flush();
    }
}

void LogStream::_Put(int w) {
    CriticalSection::Lock __(cs);
    Put0(w);
}

void  LogStream::_Put(const void *data, dword size) {
    CriticalSection::Lock __(cs);
    const byte *q = (byte *)data;
    while(size--)
        Put0(*q++);
}

bool LogStream::IsOpen() const {
    return hfile != INVALID_HANDLE_VALUE;
}

static void sLarge(String& text, size_t *large, int count, const char *txt) {
    int n = min(1024, count);
    Sort(large, large + n, StdLess<size_t>());
    int i = 0;
    while(i < n) {
        size_t q = large[i];
        int nn = i++;
        while(i < n && large[i] == q) i++;
        nn = i - nn;
        if(q < 10000)
            text << Format("%4d B, %5d %s (%6d KB)\r\n", (int)(uintptr_t)q, nn, txt, (int)(uintptr_t)((nn * q) >> 10));
        else
            text << Format("%4d`KB, %5d %s (%6d KB)\r\n", (int)(uintptr_t)(q >> 10), nn, txt, (int)(uintptr_t)((nn * q) >> 10));
    }
}

String AsString(MemoryProfile& mem) {
    String text;
    int acount = 0;
    size_t asize = 0;
    int fcount = 0;
    size_t fsize = 0;
    for(int i = 0; i < 1024; i++)
        if(mem.allocated[i]) {
            int sz = 4 * i;
            text << Format("%4d B, %6d allocated (%5d KB), %6d fragmented (%5d KB)\n",
                           sz, mem.allocated[i], (mem.allocated[i] * sz) >> 10,
                           mem.fragmented[i], (mem.fragmented[i] * sz) >> 10);
            acount += mem.allocated[i];
            asize += mem.allocated[i] * sz;
            fcount += mem.fragmented[i];
            fsize += mem.fragmented[i] * sz;
        }
    text << Format(" TOTAL, %6d allocated (%5d KB), %6d fragmented (%5d KB)\n",
                   acount, int(asize >> 10), fcount, int(fsize >> 10));
    text << "Free pages " << mem.freepages << " (" << mem.freepages * 4 << " KB)\n";
    text << "Large block count " << mem.large_count
         << ", total size " << (mem.large_total >> 10) << " KB\n";
//  sLarge(text, mem.large_size, mem.large_count, "allocated");
    text << "Large fragments count " << mem.large_free_count
         << ", total size " << (mem.large_free_total >> 10) << " KB\n";
//  sLarge(text, mem.large_free_size, mem.large_free_count, "fragments");
    return text;
}


#ifdef _MULTITHREADED

StaticCriticalSection sLogLock;

void LockLog() {
    sLogLock.Enter();
}

void UnlockLog() {
    sLogLock.Leave();
}

#endif

#ifdef flagCHECKINIT

void InitBlockBegin__(const char *fn, int line) {
    RLOG(fn << " " << line << " init block");
#ifdef HEAPDBG
    MemoryCheckDebug();
#else
    MemoryCheck();
#endif
}

void InitBlockEnd__(const char *fn, int line) {
    RLOG(fn << " " << line << " init block finished");
#ifdef HEAPDBG
    MemoryCheckDebug();
#else
    MemoryCheck();
#endif
}

#endif

END_UPP_NAMESPACE
