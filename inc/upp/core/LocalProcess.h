
struct AProcess : NoCopy {
public:
    virtual void Kill() = 0;
    virtual bool IsRunning() = 0;
    virtual void Write(String s) = 0;
    virtual bool Read(String& s) = 0;
    virtual int  GetExitCode() = 0;
    virtual void Detach()          {};

    String  Get() {
        String x;
        if(Read(x)) return x;
        return String::GetVoid();
    }

    AProcess() {}
    virtual ~AProcess() {}
};

class LocalProcess : public AProcess {
public:
    virtual void Kill();
    virtual bool IsRunning();
    virtual void Write(String s);
    virtual bool Read(String& s);
    virtual int  GetExitCode();
    virtual void Detach();

private:
    void         Init();
    void         Free();


private:
    HANDLE       hProcess;
    HANDLE       hOutputRead;
    HANDLE       hInputWrite;

    int          exit_code;

    typedef LocalProcess CLASSNAME;

public:
    bool Start(const char *cmdline, const char *envptr = NULL);

    LocalProcess() {
        Init();
    }
    LocalProcess(const char *cmdline, const char *envptr = NULL) {
        Init();
        Start(cmdline, envptr);
    }
    virtual ~LocalProcess() {
        Kill();
    }
};

int    Sys(const char *cmd, String& output);
String Sys(const char *cmd);
