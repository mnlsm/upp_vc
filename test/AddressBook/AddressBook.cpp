// This is an upp conversion of qt example, see
// http://doc.trolltech.com/3.0/addressbook-example.html
#include "stdafx.h"
#include <CtrlLib/CtrlLib.h>
#include <Report/Report.h>
#include <core/core.h>
#include <draw/draw.h>


#include "uppex/window/UppSkinWnd.h"
#include "uppex/UppSkinMgr.h"
using namespace Upp;

#define LAYOUTFILE <AddressBook/AddressBook.lay>
#include <CtrlCore/lay.h>

class AddressBook : public WithAddressBookLayout<TopWindow> {
    WithModifyLayout<ParentCtrl> modify;
    WithSearchLayout<ParentCtrl> search;
    FileSel fs;
    String  filename;

    void SetupSearch();
    void Add();
    void Change();
    void Search();
    void Open();
    void Save();
    void SaveAs();
    void Print();
    void Quit();
    void FileMenu(Bar& bar);
    void MainMenu(Bar& bar);

    typedef AddressBook CLASSNAME;
public:
    virtual LRESULT  WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

public:
    virtual void Paint(Draw& w);
    virtual void LeftDown(Point p, dword keyflags);
    virtual void Layout();


public:
    void Serialize(Stream& s);

    AddressBook();

private:
    Upp::Image img_test_;
};

void AddressBook::Layout() {
    modify.SetRect(tab.GetView());
    search.SetRect(tab.GetView());

}

AddressBook::AddressBook() {
    String skinpath = GetModuleFileNameA(NULL);
    skinpath = Upp::AppendFileName(GetFileFolder(skinpath) , FromSystemCharset("skins\\默认\\res.xml"));
    theSkinMgr.LoadSkinFromXmlFile(skinpath  , true);
    //modify.add.SetStyle(theSkinMgr.GetButtonStyle("normal"));

    CtrlLayout(*this, "Address book");
    CtrlLayout(modify);
    CtrlLayout(search);
    tab.Add(modify, "Modify");
    tab.Add(search, "Search");
    ActiveFocus(search.name);
    search.oname = true;
    search.oname <<= search.osurname <<= search.oaddress
                                         <<= search.oemail <<= THISBACK(SetupSearch);
    array.AddColumn("Name");
    array.AddColumn("Surname");
    array.AddColumn("Address");
    array.AddColumn("Email");
    modify.add <<= THISBACK(Add);
    modify.change <<= THISBACK(Change);
    search.search <<= THISBACK(Search);
    SetupSearch();
    fs.AllFilesType();
    menu.Set(THISBACK(MainMenu));

    img_test_ = Upp::StreamRaster::LoadFileAny("e:\\mid_73539098.jpg");
    MaximizeBox(true);
    FrameLess();

    array.MultiSelect(false);


}

void AddressBook::FileMenu(Bar& bar) {
    bar.Add("Open..", CtrlImg::open(), THISBACK(Open));
    bar.Add("Save", CtrlImg::save(), THISBACK(Save));
    bar.Add("Save as..", CtrlImg::save_as(), THISBACK(SaveAs));
    bar.Separator();
    bar.Add("Print", CtrlImg::print(), THISBACK(Print));
    bar.Separator();
    bar.Add("Quit", THISBACK(Quit));
}

void AddressBook::MainMenu(Bar& bar) {
    bar.Add("File", THISBACK(FileMenu));
}

void AddressBook::SetupSearch() {
    search.name.Enable(search.oname);
    search.surname.Enable(search.osurname);
    search.address.Enable(search.oaddress);
    search.email.Enable(search.oemail);
}

void AddressBook::Add() {
    array.Add(~modify.name, ~modify.surname, ~modify.address, ~modify.email);
    array.GoEnd();
    modify.name <<= modify.surname <<= modify.address <<= modify.email <<= Null;
    ActiveFocus(modify.name);
}

void AddressBook::Change() {
    if(array.IsCursor()) {
        array.Set(0, ~modify.name);
        array.Set(1, ~modify.surname);
        array.Set(2, ~modify.address);
        array.Set(3, ~modify.email);
    }
}

bool Contains(const String& text, const String& substr) {
    for(const char *s = text; s <= text.End() - substr.GetLength(); s++)
        if(strncmp(s, substr, substr.GetLength()) == 0)
            return true;
    return false;
}

void AddressBook::Search() {
    if(!array.GetCount()) return;
    bool sc = true;
    array.ClearSelection();
    for(int i = 0; i < array.GetCount(); i++) {
        if((!search.oname || Contains(array.Get(i, 0), ~search.name)) &&
                (!search.osurname || Contains(array.Get(i, 1), ~search.surname)) &&
                (!search.oaddress || Contains(array.Get(i, 2), ~search.address)) &&
                (!search.oemail || Contains(array.Get(i, 3), ~search.email))) {
            array.Select(i);
            if(sc) {
                array.SetCursor(i);
                array.CenterCursor();
                sc = false;
            }
            array.Refresh();
        }
    }
}

void AddressBook::Open() {
    if(!fs.ExecuteOpen()) return;
    filename = fs;
    FileIn in(filename);
    if(!in) {
        Exclamation("Unable to open [* " + DeQtf(filename));
        return;
    }
    array.Clear();
    while(!in.IsEof()) {
        Vector<Value> q;
        for(int i = 0; i < 4; i++)
            q.Add(in.GetLine());
        array.Add(q);
    }
}

void AddressBook::Save() {
    if(IsEmpty(filename)) {
        SaveAs();
        return;
    }
    FileOut out(filename);
    if(!out) {
        Exclamation("Unable to open " + filename);
        return;
    }
    for(int i = 0; i < array.GetCount(); i++)
        for(int q = 0; q < 4; q++)
            out.PutLine(String(array.Get(i, q)));
}

void AddressBook::SaveAs() {
    if(!fs.ExecuteSaveAs()) return;
    filename = fs;
    Save();
}


void AddressBook::Print() {
    String qtf;
    qtf = "{{1:1:1:1 Name:: Surname:: Address:: Email";
    for(int i = 0; i < array.GetCount(); i++)
        for(int q = 0; q < 4; q++)
            qtf << ":: " << DeQtf((String)array.Get(i, q));
    Report report;
    report << qtf;
    Perform(report);
}

void AddressBook::Quit() {
    Break();
}

void AddressBook::Serialize(Stream& s) {
    int version = 0;
    s / version;
    s % search.oname % search.osurname % search.oaddress % search.oemail;
    s % fs;
    SetupSearch();
}

void AddressBook::Paint(Draw& w) {
    TopWindow::Paint(w);
    w.DrawImage(50, 100 , img_test_);
}

void AddressBook::LeftDown(Point p, dword keyflags) {
    //PostMessage( GetHWND() , WM_NCLBUTTONDOWN , HTCAPTION , 0 );
}

#define TOPBORDER 4
#define BOTTOMBORDER 4
#define LEFTBORDER 4
#define RIGHTBORDER 4
LRESULT AddressBook::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
    switch(message) {
    case WM_NCPAINT:
        return 0L;
    case WM_NCCALCSIZE:
        return 0L;
    case WM_NCHITTEST:
        //TopWindow::WindowProc( message, wParam, lParam );
        RECT rc;
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(GetHWND() , &pt);
        GetClientRect(GetHWND() , &rc);
        if(pt.x < LEFTBORDER && pt.y < TOPBORDER)
            return HTTOPLEFT;
        else if(pt.x < LEFTBORDER && pt.y > rc.bottom - BOTTOMBORDER)
            return HTBOTTOMLEFT;
        else if(pt.x > rc.right - RIGHTBORDER && pt.y < TOPBORDER)
            return HTTOPRIGHT;
        else if(pt.x > rc.right - RIGHTBORDER && pt.y > rc.bottom - BOTTOMBORDER)
            return HTBOTTOMRIGHT;
        else if(pt.x < LEFTBORDER)
            return HTLEFT;
        else if(pt.x > rc.right - RIGHTBORDER)
            return HTRIGHT;
        else if(pt.y < TOPBORDER)
            return HTTOP;
        else if(pt.y > rc.bottom - BOTTOMBORDER)
            return HTBOTTOM;
        else if(pt.y < 20)
            return HTCAPTION;
        return HTCLIENT;
    case WM_NCLBUTTONDOWN:
    case WM_NCRBUTTONDOWN:
    case WM_NCMBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_NCMOUSEMOVE:
    case WM_NCLBUTTONUP:
    case WM_NCLBUTTONDBLCLK:
    case WM_NCRBUTTONUP:
    case WM_NCRBUTTONDBLCLK:
    case WM_NCMBUTTONUP:
    case WM_NCMBUTTONDBLCLK:
        GetCursorPos(&pt);
        ScreenToClient(GetHWND() , &pt);
        WORD dwKeyState ;
        LRESULT lRet ;
        dwKeyState = 0;
        if(GetCtrl()) dwKeyState |= MK_CONTROL;
        if(GetShift()) dwKeyState |= MK_SHIFT;
        if(GetMouseLeft()) dwKeyState |= MK_LBUTTON;
        if(GetMouseRight()) dwKeyState |= MK_RBUTTON;
        if(GetMouseMiddle()) dwKeyState |= MK_MBUTTON;
        if(!!(GetKeyState(VK_XBUTTON1) & 0x8000)) dwKeyState |= MK_XBUTTON1;
        if(!!(GetKeyState(VK_XBUTTON2) & 0x8000)) dwKeyState |= MK_XBUTTON2;
        lRet = SendMessage(GetHWND() , WM_MOUSEMOVE - WM_NCMOUSEMOVE + message , MAKELPARAM(dwKeyState , 0) , MAKELPARAM(pt.x , pt.y));
        if(HasMouseDeep() && GetMouseCtrl() != this)
            return lRet;
        return DefWindowProc(GetHWND() , message, wParam, lParam);
        break;
    case WM_NCMOUSEHOVER:
    case WM_NCMOUSELEAVE:
        GetCursorPos(&pt);
        ScreenToClient(GetHWND() , &pt);
        SendMessage(GetHWND() , message == WM_NCMOUSEHOVER ? WM_MOUSEHOVER : WM_MOUSELEAVE , wParam , MAKELPARAM(pt.x , pt.y));
        return DefWindowProc(GetHWND() , message, wParam, lParam);
        break;


    default:
        break;
    }
    return TopWindow::WindowProc(message, wParam, lParam);
}

void OpenPEFile(LPSTR lpExeName, LPSTR lpDllName);
int AddImportDll(HANDLE hFile, DWORD dwBase, PIMAGE_NT_HEADERS pNTHeader);


#define PROCESSOR_FEATURE_MAX 64
#define MAXIMUM_XSTATE_FEATURES             (64)

typedef struct _KSYSTEM_TIME {
    ULONG LowPart;
    LONG High1Time;
    LONG High2Time;
} KSYSTEM_TIME, *PKSYSTEM_TIME;

typedef enum _NT_PRODUCT_TYPE {
    NtProductWinNt = 1,
    NtProductLanManNt,
    NtProductServer
} NT_PRODUCT_TYPE, *PNT_PRODUCT_TYPE;

typedef enum _ALTERNATIVE_ARCHITECTURE_TYPE {
    StandardDesign,                 // None == 0 == standard design
    NEC98x86,                       // NEC PC98xx series on X86
    EndAlternatives                 // past end of known alternatives
} ALTERNATIVE_ARCHITECTURE_TYPE;


typedef struct _XSTATE_FEATURE {
    ULONG Offset;
    ULONG Size;
} XSTATE_FEATURE, *PXSTATE_FEATURE;

typedef struct _XSTATE_CONFIGURATION {
    // Mask of all enabled features
    ULONG64 EnabledFeatures;

    // Mask of volatile enabled features
    ULONG64 EnabledVolatileFeatures;

    // Total size of the save area for user states
    ULONG Size;

    // Control Flags
    ULONG OptimizedSave : 1;
    ULONG CompactionEnabled : 1;

    // List of features
    XSTATE_FEATURE Features[MAXIMUM_XSTATE_FEATURES];

    // Mask of all supervisor features
    ULONG64 EnabledSupervisorFeatures;

    // Mask of features that require start address to be 64 byte aligned
    ULONG64 AlignedFeatures;

    // Total size of the save area for user and supervisor states
    ULONG AllFeatureSize;

    // List which holds size of each user and supervisor state supported by CPU        
    ULONG AllFeatures[MAXIMUM_XSTATE_FEATURES];

} XSTATE_CONFIGURATION, *PXSTATE_CONFIGURATION;


typedef struct _KUSER_SHARED_DATA {
  ULONG                         TickCountLowDeprecated;
  ULONG                         TickCountMultiplier;
  KSYSTEM_TIME                  InterruptTime;
  KSYSTEM_TIME                  SystemTime;
  KSYSTEM_TIME                  TimeZoneBias;
  USHORT                        ImageNumberLow;
  USHORT                        ImageNumberHigh;
  WCHAR                         NtSystemRoot[260];
  ULONG                         MaxStackTraceDepth;
  ULONG                         CryptoExponent;
  ULONG                         TimeZoneId;
  ULONG                         LargePageMinimum;
  ULONG                         AitSamplingValue;
  ULONG                         AppCompatFlag;
  ULONGLONG                     RNGSeedVersion;
  ULONG                         GlobalValidationRunlevel;
  LONG                          TimeZoneBiasStamp;
  ULONG                         NtBuildNumber;
  NT_PRODUCT_TYPE               NtProductType;
  BOOLEAN                       ProductTypeIsValid;
  BOOLEAN                       Reserved0[1];
  USHORT                        NativeProcessorArchitecture;
  ULONG                         NtMajorVersion;
  ULONG                         NtMinorVersion;
  BOOLEAN                       ProcessorFeatures[PROCESSOR_FEATURE_MAX];
  ULONG                         Reserved1;
  ULONG                         Reserved3;
  ULONG                         TimeSlip;
  ALTERNATIVE_ARCHITECTURE_TYPE AlternativeArchitecture;
  ULONG                         BootId;
  LARGE_INTEGER                 SystemExpirationDate;
  ULONG                         SuiteMask;
  BOOLEAN                       KdDebuggerEnabled;
  union {
    UCHAR MitigationPolicies;
    struct {
      UCHAR NXSupportPolicy : 2;
      UCHAR SEHValidationPolicy : 2;
      UCHAR CurDirDevicesSkippedForDlls : 2;
      UCHAR Reserved : 2;
    };
  };
  USHORT                        CyclesPerYield;
  ULONG                         ActiveConsoleId;
  ULONG                         DismountCount;
  ULONG                         ComPlusPackage;
  ULONG                         LastSystemRITEventTickCount;
  ULONG                         NumberOfPhysicalPages;
  BOOLEAN                       SafeBootMode;
  union {
    UCHAR VirtualizationFlags;
    struct {
      UCHAR ArchStartedInEl2 : 1;
      UCHAR QcSlIsSupported : 1;
    };
  };
  UCHAR                         Reserved12[2];
  union {
    ULONG SharedDataFlags;
    struct {
      ULONG DbgErrorPortPresent : 1;
      ULONG DbgElevationEnabled : 1;
      ULONG DbgVirtEnabled : 1;
      ULONG DbgInstallerDetectEnabled : 1;
      ULONG DbgLkgEnabled : 1;
      ULONG DbgDynProcessorEnabled : 1;
      ULONG DbgConsoleBrokerEnabled : 1;
      ULONG DbgSecureBootEnabled : 1;
      ULONG DbgMultiSessionSku : 1;
      ULONG DbgMultiUsersInSessionSku : 1;
      ULONG DbgStateSeparationEnabled : 1;
      ULONG SpareBits : 21;
    } DUMMYSTRUCTNAME2;
  } DUMMYUNIONNAME2;
  ULONG                         DataFlagsPad[1];
  ULONGLONG                     TestRetInstruction;
  LONGLONG                      QpcFrequency;
  ULONG                         SystemCall;
  ULONG                         Reserved2;
  ULONGLONG                     SystemCallPad[2];
  union {
    KSYSTEM_TIME TickCount;
    ULONG64      TickCountQuad;
    struct {
      ULONG ReservedTickCountOverlay[3];
      ULONG TickCountPad[1];
    } DUMMYSTRUCTNAME;
  } DUMMYUNIONNAME3;
  ULONG                         Cookie;
  ULONG                         CookiePad[1];
  LONGLONG                      ConsoleSessionForegroundProcessId;
  ULONGLONG                     TimeUpdateLock;
  ULONGLONG                     BaselineSystemTimeQpc;
  ULONGLONG                     BaselineInterruptTimeQpc;
  ULONGLONG                     QpcSystemTimeIncrement;
  ULONGLONG                     QpcInterruptTimeIncrement;
  UCHAR                         QpcSystemTimeIncrementShift;
  UCHAR                         QpcInterruptTimeIncrementShift;
  USHORT                        UnparkedProcessorCount;
  ULONG                         EnclaveFeatureMask[4];
  ULONG                         TelemetryCoverageRound;
  USHORT                        UserModeGlobalLogger[16];
  ULONG                         ImageFileExecutionOptions;
  ULONG                         LangGenerationCount;
  ULONGLONG                     Reserved4;
  ULONGLONG                     InterruptTimeBias;
  ULONGLONG                     QpcBias;
  ULONG                         ActiveProcessorCount;
  UCHAR                         ActiveGroupCount;
  UCHAR                         Reserved9;
  union {
    USHORT QpcData;
    struct {
      UCHAR QpcBypassEnabled;
      UCHAR QpcShift;
    };
  };
  LARGE_INTEGER                 TimeZoneBiasEffectiveStart;
  LARGE_INTEGER                 TimeZoneBiasEffectiveEnd;
  XSTATE_CONFIGURATION          XState;
  KSYSTEM_TIME                  FeatureConfigurationChangeStamp;
  ULONG                         Spare;
  ULONG64                       UserPointerAuthMask;
} KUSER_SHARED_DATA, *PKUSER_SHARED_DATA;

GUI_APP_MAIN {
    //int index = Font::FindFaceNameIndex( WString( L"宋体" ).ToString() );
    //SetStdFont( Font( index , 12 ) );
    //static char sZoomText[] = "OK Cancel Exit Retry";
    //Size sz = GetTextSize( sZoomText , Font( index , 16 ) );
    //Size sz1 = sz;
    //Ctrl::SetZoomSize( sz , sz1 );
//   AddressBook ab;
//   LoadFromFile( ab );
//   ab.Run();
//   StoreToFile( ab );
    //SetWindowPos
    //WM_KILLFOCUS
    using namespace Uppex;
    InitializeUppCtrlCreators();
    WCHAR szVerifyPath[MAX_PATH];
    int a = sizeof(szVerifyPath);
	PKUSER_SHARED_DATA s = (PKUSER_SHARED_DATA)(0x7FFE0000);

    String path = GetModuleFileNameA(NULL);
    String skinpath = Upp::AppendFileName(GetFileFolder(path) , FromSystemCharset("skins\\默认\\res.xml"));
    theSkinMgr.LoadSkinFromXmlFile(skinpath  , true);

    String respath = Upp::AppendFileName(GetFileFolder(path), FromSystemCharset("skins\\默认"));
    String outpath = Upp::AppendFileName(GetFileFolder(respath), FromSystemCharset("default.mrs"));

    theSkinMgr.CreateSkinToMarisaFile(respath, outpath);
    theSkinMgr.LoadSkinFromMarisaFile(~outpath);

    String main_path = Upp::AppendFileName(GetFileFolder(path) , FromSystemCharset("skins\\默认\\mainwnd.xml"));

    if(true) {
        CUppSkinWnd wnd;
        //wnd.BuildFromXmlFile(main_path , true);
        wnd.BuildFromSkinLayout("layout/mainwnd.xml");
        wnd.Run();
    }
    return ;
}

//struct MyApp : TopWindow {
//
//  Image image;
//
//
//  void Paint(Draw& w)
//  {
//
//      w.DrawRect(GetSize(), Cyan());
//
//      w.DrawImage(10, 10, image);
//
//      Image img = theSkinMgr.ExtractImage("sysbar_close_normal");
//      int width = img.GetWidth();
//      w.DrawImage( 50 ,80 , img );
//
//  }
//
//
//  MyApp()
//  {
//      ImageDraw iw(100, 40);
//      iw.Alpha().DrawRect(0, 0, 100, 40, Color(0,11,11));
//      iw.Alpha().DrawEllipse(0, 0, 100, 40, Color(255,66,255));
//      iw.DrawEllipse(0, 0, 100, 40, Yellow());
//      iw.DrawText(26, 10, "Image", Arial(16).Bold());
//      image = iw;
//      String skinpath = GetModuleFileNameA( NULL );
//      skinpath = Upp::AppendFileName( GetFileFolder( skinpath ) , FromSystemCharset( "skins\\默认\\res.xml" ) );
//      theSkinMgr.LoadSkinFromXmlFile( FromSystemCharset( "默认" ) , skinpath  , true );
//  }
//
//};
//
//
//
//GUI_APP_MAIN
//{
//  CUppSkinWnd wnd;
//
//  BOOL b = wnd.BuildFromXmlFile( "f:\\skinwnd.xml" , true );
//
//
//  TopWindow app0;
//  app0.NoCloseBox(true).FrameLess(false).SetEditable(false).Enable( false );
//  app0.Show( false );
//  app0.SetRect( 0 , 0 , 1 , 1 );
//  app0.OpenMain();
//
//  MyApp app ;
//  app.SetRect( 0 , 0 , 300, 300 );
//  app.Open( &app0 );
//  app.SetForeground();
//  app.CenterScreen().Sizeable().Run();
//  app0.Close();
//
//}
//




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OpenPEFile(LPSTR lpExeName, LPSTR lpDllName) {
    HANDLE hFile;
    HANDLE hFileMapping;
    LPVOID lpFileBase;
    PIMAGE_DOS_HEADER    dosHeader;
    PIMAGE_NT_HEADERS    pNTHeader;


    char szNewFileName[MAX_PATH];
    sprintf(szNewFileName, "%s", lpExeName);
    strcat(szNewFileName, ".bak");
    if(!CopyFileA(lpExeName, szNewFileName, FALSE)) {
        fprintf(stderr, "CopyFile() failed. --err: %d\n", GetLastError());
        return;
    }
    //    printf("lpFileName: %s lpNewFileName: %s\n", lpFileName, lpNewFileName);

    hFile = CreateFileA(szNewFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if(hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateFile() failed. --err: %d\n", GetLastError());
        return;
    }

    hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if(hFileMapping == 0) {
        CloseHandle(hFile);
        printf("Couldn't open file mapping with CreateFileMapping()\n");
        return;
    }

    lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if(lpFileBase == 0) {
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        printf("Couldn't map view of file with MapViewOfFile()\n");
        return;
    }


    dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
    pNTHeader = (IMAGE_NT_HEADERS *)((BYTE *) lpFileBase + dosHeader->e_lfanew);

    if(dosHeader->e_magic == IMAGE_DOS_SIGNATURE && pNTHeader->Signature == 0x4550) {
        AddImportDll(hFile, (DWORD)dosHeader, pNTHeader);
    }else
        printf("unrecognized file format\n");

    UnmapViewOfFile(lpFileBase);
    CloseHandle(hFileMapping);
    CloseHandle(hFile);
}




#define IMAGE_FIRST_SECTION32( ntheader ) ((PIMAGE_SECTION_HEADER)  ((uint64*)ntheader + FIELD_OFFSET( IMAGE_NT_HEADERS32, OptionalHeader ) + ((PIMAGE_NT_HEADERS32)(ntheader))->FileHeader.SizeOfOptionalHeader   ))


PIMAGE_SECTION_HEADER
GetEnclosingSectionHeader(
    DWORD rva,
    PIMAGE_NT_HEADERS pNTHeader
) {
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION32(pNTHeader);
    unsigned i;

    for(i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++) {
        // Is the RVA within this section?
        if((rva >= section->VirtualAddress) &&
                (rva < (section->VirtualAddress + section->Misc.VirtualSize)))
            return section;
    }

    return section;
}


int AddImportDll(HANDLE hFile, DWORD dwBase, PIMAGE_NT_HEADERS pNTHeader) {
    //
    // 通过OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress
    // 获得导入表的RVA, 利用此RVA找到ImportTable所在的Section,之后计算Offset,公式:
    // Offset = (INT)(pSection->VirtualAddress - pSection->PointerToRawData)
    // 之后利用Offset来定位文件中ImportTable的位置.
    //
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = 0;
    PIMAGE_SECTION_HEADER pSection = 0;
    PIMAGE_THUNK_DATA pThunk, pThunkIAT = 0;
    int Offset = -1;

    pSection = GetEnclosingSectionHeader(pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, pNTHeader);
    if(!pSection) {
        fprintf(stderr, "No Import Table..\n");
        return -1;
    }

    Offset = (int)(pSection->VirtualAddress - pSection->PointerToRawData);

    //
    // 计算ImportTable在文件中的位置
    //
    pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)(pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress - Offset + dwBase);

    //
    // 取出导入的DLL的个数
    //
    int nImportDllCount = 0;
    while(1) {
        if((pImportDesc->TimeDateStamp == 0) && (pImportDesc->Name == 0))
            break;
        pThunk = (PIMAGE_THUNK_DATA)(pImportDesc->Characteristics);
        pThunkIAT = (PIMAGE_THUNK_DATA)(pImportDesc->FirstThunk);

        if(pThunk == 0 && pThunkIAT == 0)
            return -1;

        nImportDllCount++;
        pImportDesc++;
    }

    //
    // 恢复pImportDesc的值,方便下面的复制当前导入表的操作.
    //
    pImportDesc -= nImportDllCount;

    //
    // 取得ImportTable所在Section的RawData在文件中的末尾地址,计算公式:
    // dwOrigEndOfRawDataAddr = pSection->PointerToRawData + pSection->Misc.VirtualSize
    //
    DWORD dwEndOfRawDataAddr = pSection->PointerToRawData + pSection->Misc.VirtualSize;

    PIMAGE_IMPORT_DESCRIPTOR pImportDescVector =
        (PIMAGE_IMPORT_DESCRIPTOR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 20 * (nImportDllCount + 1));
    if(pImportDescVector == NULL) {
        fprintf(stderr, "HeapAlloc() failed. --err: %d\n", GetLastError());
        return -1;
    }
    CopyMemory(pImportDescVector + 1, pImportDesc, 20 * nImportDllCount);


    //
    // 构造添加数据的结构,方法笨拙了点.
    //
    struct _Add_Data {
        char szDllName[256]; // 导入DLL的名字
        int nDllNameLen; // 实际填充的名字的长度
        WORD Hint; // 导入函数的Hint
        char szFuncName[256]; // 导入函数的名字
        int nFuncNameLen; // 导入函数名字的实际长度
        int nTotal; // 填充的总长度
    } Add_Data;
    const char szDll[256] = "QQ_main.dll";
    const char szFunc[256] = "SoftWareGoIn";
    strcpy(Add_Data.szDllName, szDll);
    strcpy(Add_Data.szFuncName, szFunc);

    //
    // +1表示'\0'字符
    //
    Add_Data.nDllNameLen = strlen(Add_Data.szDllName) + 1;
    Add_Data.nFuncNameLen = strlen(Add_Data.szFuncName) + 1;
    Add_Data.Hint = 0;
    //
    // 计算总的填充字节数
    //
    Add_Data.nTotal = Add_Data.nDllNameLen + sizeof(WORD) + Add_Data.nFuncNameLen;

    //
    // 检查ImportTable所在的Section中的剩余空间是否能够容纳新的ImportTable.
    // 未对齐前RawData所占用的空间存放在pSection->VirtualSize中,用此值加上新的ImportTable长度与
    // 原长度进行比较.
    //
    // nTotalLen 为新添加内容的总长度
    // Add_Data.nTotal 为添加的DLL名称,Hint与导入函数的名字的总长度.
    // 8 为IMAGE_IMPORT_BY_NAME结构以及保留空的长度.
    // 20*(nImportDllCount+1) 为新的ImportTable的长度.
    //
    int nTotalLen = Add_Data.nTotal + 8 + 20 * (nImportDllCount + 1);
    printf("TotalLen: %d byte(s)\n", nTotalLen);
    if(pSection->Misc.VirtualSize + nTotalLen > pSection->SizeOfRawData) {
        fprintf(stderr, "No enough space!\n");
        return -1;
    }

    IMAGE_IMPORT_DESCRIPTOR Add_ImportDesc;
    //
    // ThunkData结构的地址
    //
    Add_ImportDesc.Characteristics = dwEndOfRawDataAddr + Add_Data.nTotal + Offset;
    Add_ImportDesc.TimeDateStamp = -1;
    Add_ImportDesc.ForwarderChain = -1;
    //
    // DLL名字的RVA
    //
    Add_ImportDesc.Name = dwEndOfRawDataAddr + Offset;
    Add_ImportDesc.FirstThunk = Add_ImportDesc.Characteristics;

    CopyMemory(pImportDescVector, &Add_ImportDesc, 20);

    //
    // 对文件进行修改
    //
    DWORD dwBytesWritten = 0;
    DWORD dwBuffer = dwEndOfRawDataAddr + Offset + Add_Data.nTotal + 8;
    long lDistanceToMove = (long) & (pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) - dwBase;
    int nRet = 0;

    //
    // 修改IMAGE_DIRECTOR_ENTRY_IMPORT中VirtualAddress的地址,
    // 使其指向新的导入表的位置
    //
    SetFilePointer(hFile, lDistanceToMove, NULL, FILE_BEGIN);

    printf("OrigEntryImport: %x\n", pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    nRet = WriteFile(hFile, (PVOID)&dwBuffer, 4, &dwBytesWritten, NULL);
    if(!nRet) {
        fprintf(stderr, "WriteFile(ENTRY_IMPORT) failed. --err: %d\n", GetLastError());
        return -1;
    }
    printf("NewEntryImport: %x\n", pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    //
    // 修改导入表长度,这个部分具体要修改为多少我也没弄明白,不过按照测试,改与不改都可以工作
    //
    dwBuffer = pNTHeader->OptionalHeader.DataDirectory
               [IMAGE_DIRECTORY_ENTRY_IMPORT].Size + 40;
    nRet = WriteFile(hFile, (PVOID)&dwBuffer, 4, &dwBytesWritten, NULL);
    if(!nRet) {
        fprintf(stderr, "WriteFile(Entry_import.size) failed. --err: %d\n", GetLastError());
        return -1;
    }

    //
    // 修改ImportTable所在节的长度
    //
    lDistanceToMove = (long) & (pSection->Misc.VirtualSize) - dwBase;
    SetFilePointer(hFile, lDistanceToMove, NULL, FILE_BEGIN);
    dwBuffer = pSection->Misc.VirtualSize + nTotalLen;
    nRet = WriteFile(hFile, (PVOID)&dwBuffer, 4, &dwBytesWritten, NULL);
    if(!nRet) {
        fprintf(stderr, "WriteFile(Misc.VirtualSize) failed. --err: %d\n", GetLastError());
        return -1;
    }

    //
    // 从节的末尾添加新的DLL内容
    // 偷点懒,返回值就不检查了..
    //
    lDistanceToMove = dwEndOfRawDataAddr;
    SetFilePointer(hFile, lDistanceToMove, NULL, FILE_BEGIN);
    nRet = WriteFile(hFile, Add_Data.szDllName, Add_Data.nDllNameLen, &dwBytesWritten, NULL);
    nRet = WriteFile(hFile, (LPVOID) & (Add_Data.Hint), sizeof(WORD), &dwBytesWritten, NULL);
    nRet = WriteFile(hFile, Add_Data.szFuncName, Add_Data.nFuncNameLen, &dwBytesWritten, NULL);
    dwBuffer = dwEndOfRawDataAddr + Add_Data.nDllNameLen + Offset;
    nRet = WriteFile(hFile, (LPVOID)&dwBuffer, 4, &dwBytesWritten, NULL);
    dwBuffer = 0;
    nRet = WriteFile(hFile, (LPVOID)&dwBuffer, 4, &dwBytesWritten, NULL);
    nRet = WriteFile(hFile, (LPVOID)pImportDescVector, 20 * (nImportDllCount + 1), &dwBytesWritten, NULL);

    HeapFree(GetProcessHeap(), 0, pImportDescVector);
    return 0;
}