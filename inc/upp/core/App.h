
String  GetEnv(const char *id);

String  GetExeFilePath();
String  GetExeDirFile(const char *fp);

String  GetHomeDirFile(const char *fp);
String  GetHomeDirectory();



String  GetExeTitle();

void    UseHomeDirectoryConfig(bool b = true);

String  ConfigFile(const char *file);
String  ConfigFile();

const Vector<String>& CommandLine();
const VectorMap<String, String>& Environment();

void    SetExitCode(int code);
int     GetExitCode();

bool    IsMainRunning();

#ifndef flagSO
//void    Main(); // By console application
#endif

void AppExit__();


void AppInit__(int argc, const char **argv);
void AppInitEnvironment__();

#define CONSOLE_APP_MAIN \
void ConsoleMainFn_(); \
 \
int main(int argc, char *argv[]) { \
    UPP::AppInit__(argc, (const char **)argv); \
    ConsoleMainFn_(); \
    UPP::DeleteUsrLog(); \
    UPP::AppExit__(); \
    return UPP::GetExitCode(); \
} \
 \
void ConsoleMainFn_()




String  GetDataFile(const char *filename);

void    LaunchWebBrowser(const String& url);

String GetComputerName();
String GetUserName();
String GetDesktopManager();

String GetDesktopFolder();
String GetProgramsFolder();
String GetAppDataFolder();
String GetMusicFolder();
String GetPicturesFolder();
String GetVideoFolder();
String GetDocumentsFolder();
String GetTemplatesFolder();
String GetDownloadFolder();
