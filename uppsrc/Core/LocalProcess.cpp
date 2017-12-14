#include "Core.h"

NAMESPACE_UPP



#define LLOG(x) // LOG(x)

void LocalProcess::Init() {
    hProcess = hOutputRead = hInputWrite = NULL;
    exit_code = Null;
}

void LocalProcess::Free() {
    if(hProcess) {
        CloseHandle(hProcess);
        hProcess = NULL;
    }
    if(hOutputRead) {
        CloseHandle(hOutputRead);
        hOutputRead = NULL;
    }
    if(hInputWrite) {
        CloseHandle(hInputWrite);
        hInputWrite = NULL;
    }
    exit_code = Null;
}

bool LocalProcess::Start(const char *command, const char *envptr) {
    LLOG("LocalProcess::Start(\"" << command << "\")");

    Kill();

    while(*command && (byte)*command <= ' ')
        command++;


    HANDLE hOutputReadTmp, hInputRead;
    HANDLE hInputWriteTmp, hOutputWrite;
    HANDLE hErrorWrite;
    SECURITY_ATTRIBUTES sa;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    HANDLE hp = GetCurrentProcess();

    CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 0);
    DuplicateHandle(hp, hOutputWrite, hp, &hErrorWrite, 0, TRUE, DUPLICATE_SAME_ACCESS);
    CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 0);
    DuplicateHandle(hp, hOutputReadTmp, hp, &hOutputRead, 0, FALSE, DUPLICATE_SAME_ACCESS);
    DuplicateHandle(hp, hInputWriteTmp, hp, &hInputWrite, 0, FALSE, DUPLICATE_SAME_ACCESS);
    CloseHandle(hOutputReadTmp);
    CloseHandle(hInputWriteTmp);

    PROCESS_INFORMATION pi;
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdInput  = hInputRead;
    si.hStdOutput = hOutputWrite;
    si.hStdError  = hErrorWrite;
    int n = (int)strlen(command) + 1;
    Buffer<char> cmd(n);
    memcpy(cmd, command, n);
    bool h = CreateProcessA(NULL, cmd, &sa, &sa, TRUE,
                            NORMAL_PRIORITY_CLASS, (void *)envptr, NULL, &si, &pi);
    LLOG("CreateProcessA " << (h ? "succeeded" : "failed"));
    CloseHandle(hErrorWrite);
    CloseHandle(hInputRead);
    CloseHandle(hOutputWrite);
    if(h) {
        hProcess = pi.hProcess;
        CloseHandle(pi.hThread);
    }else {
        Free();
        return false;
//      throw Exc(NFormat("Error running process: %s\nCommand: %s", GetErrorMessage(GetLastError()), command));
    }
    return true;


}



void LocalProcess::Kill() {
    if(hProcess && IsRunning()) {
        TerminateProcess(hProcess, (DWORD) - 1);
        exit_code = 255;
    }
    Free();
}

void LocalProcess::Detach() {
    Free();
}

bool LocalProcess::IsRunning() {
    dword exitcode;
    if(!hProcess)
        return false;
    if(GetExitCodeProcess(hProcess, &exitcode) && exitcode == STILL_ACTIVE)
        return true;
    dword n;
    if(PeekNamedPipe(hOutputRead, NULL, 0, NULL, &n, NULL) && n)
        return true;
    exit_code = exitcode;
    return false;
}

int  LocalProcess::GetExitCode() {
    return IsRunning() ? (int)Null : exit_code;
}

bool LocalProcess::Read(String& res) {
    LLOG("LocalProcess::Read");
    res = Null;
    if(!hOutputRead) return false;
    dword n;
    if(!PeekNamedPipe(hOutputRead, NULL, 0, NULL, &n, NULL) || n == 0)
        return IsRunning();
    char buffer[1024];
    if(!ReadFile(hOutputRead, buffer, sizeof(buffer), &n, NULL))
        return false;
    res = FromSystemCharset(String(buffer, n));
    return true;
}

void LocalProcess::Write(String s) {
    dword n;
    WriteFile(hInputWrite, s, s.GetLength(), &n, NULL);
}

int Sys(const char *cmd, String& out) {
    out.Clear();
    LocalProcess p;
    if(!p.Start(cmd))
        return -1;
    while(p.IsRunning()) {
        out.Cat(p.Get());
        Sleep(1);   // p.Wait would be much better here!
    }
    out.Cat(p.Get());
    return p.GetExitCode();
}

String Sys(const char *cmd) {
    String r;
    return Sys(cmd, r) ? String::GetVoid() : r;
}

END_UPP_NAMESPACE
