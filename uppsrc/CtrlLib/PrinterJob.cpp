#include "CtrlLib.h"

#include <commdlg.h>
#pragma  comment(lib, "comdlg32.lib")



NAMESPACE_UPP





#include <commdlg.h>
#pragma  comment(lib, "comdlg32.lib")

struct Win32PrintDlg_ : PRINTDLG {
    Win32PrintDlg_() {
        memset(this, 0, sizeof(PRINTDLG));
        lStructSize = sizeof(PRINTDLG);
    }
    ~Win32PrintDlg_() {
        if(hDevMode)
            ::GlobalFree(hDevMode);
        if(hDevNames)
            ::GlobalFree(hDevNames);
    }
};

PrinterJob::PrinterJob(const char *_name) {
    name = _name;
    landscape = false;
    from = to = 1;
    current = 1;
}

PrinterJob::~PrinterJob() {
}

bool PrinterJob::Execute0(bool dodlg) {
    pdlg = new Win32PrintDlg_;
    PRINTDLG& dlg = *pdlg;
    dlg.Flags = PD_DISABLEPRINTTOFILE | PD_NOSELECTION | PD_HIDEPRINTTOFILE | PD_RETURNDEFAULT;
    dlg.nFromPage = current;
    dlg.nToPage = current;
    dlg.nMinPage = from;
    dlg.nMaxPage = to;
    if(from != to)
        dlg.Flags |= PD_ALLPAGES;
    dlg.hwndOwner = GetActiveWindow();
    dlg.Flags |= PD_RETURNDEFAULT;
    dlg.nCopies = 1;
    if(!PrintDlg(&dlg)) return false;
    if(dlg.hDevMode) {
        DEVMODE *pDevMode = (DEVMODE*)::GlobalLock(dlg.hDevMode);
        pDevMode->dmOrientation = landscape ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT;
        ::GlobalUnlock(dlg.hDevMode);
    }
    HDC hdc;
    if(dodlg) {
        dlg.Flags = PD_DISABLEPRINTTOFILE | PD_NOSELECTION | PD_HIDEPRINTTOFILE | PD_RETURNDC;
        Vector< Ptr<Ctrl> > disabled = DisableCtrls(Ctrl::GetTopCtrls());
        bool b = PrintDlg(&dlg);
        EnableCtrls(disabled);
        if(!b) return false;
        hdc = dlg.hDC;
    }else {
        DEVNAMES *p = (DEVNAMES *)::GlobalLock(dlg.hDevNames);
        const char *driver = (const char *)p + p->wDriverOffset;
        const char *device = (const char *)p + p->wDeviceOffset;
        if(dlg.hDevMode) {
            DEVMODEA *pDevMode = (DEVMODEA*)::GlobalLock(dlg.hDevMode);
            hdc = CreateDCA(driver, device, NULL, pDevMode);
            ::GlobalUnlock(dlg.hDevMode);
        }else
            hdc = CreateDCA(driver, device, NULL, NULL);
    }
    if(dlg.hDevMode)
        ::GlobalFree(dlg.hDevMode);
    if(dlg.hDevNames)
        ::GlobalFree(dlg.hDevNames);
    if(hdc) {
        draw = new PrintDraw(hdc, Nvl(name, Ctrl::GetAppName()));
        page.Clear();
        if(!(dlg.Flags & PD_PAGENUMS)) {
            dlg.nFromPage = dlg.nMinPage;
            dlg.nToPage = dlg.nMaxPage;
        }
        for(int c = 0; c < ((dlg.Flags & PD_COLLATE) ? dlg.nCopies : 1); c++)
            for(int i = dlg.nFromPage - 1; i <= dlg.nToPage - 1; i++)
                for(int c = 0; c < ((dlg.Flags & PD_COLLATE) ? 1 : dlg.nCopies); c++)
                    page.Add(i);
        return true;
    }
    return false;
}

bool PrinterJob::Execute() {
    return Execute0(true);
}

Draw& PrinterJob::GetDraw() {
    if(!draw) {
        Execute0(false);
        if(!draw)
            draw = new NilDraw;
    }
    return *draw;
}

PrinterJob& PrinterJob::MinMaxPage(int minpage, int maxpage) {
    from = minpage + 1;
    to = maxpage + 1;
    return *this;
}

PrinterJob& PrinterJob::CurrentPage(int i) {
    current = i + 1;
    return *this;
}







END_UPP_NAMESPACE
