#include "stdafx.h"
#include "SkinWnd.h"
//#include "resource.h"

using namespace Upp;

#define UPPSKINWND_TOPBORDER    4
#define UPPSKINWND_BOTTOMBORDER 4
#define UPPSKINWND_LEFTBORDER   4
#define UPPSKINWND_RIGHTBORDER  4

#define LLOG(x)
//

#define DELAY_REFRESH_MSG WM_USER + 888

NAMESPACE_UPPEX

SkinWnd::SkinWnd()
    : Upp::TopWindow() {
    topborder_ = UPPSKINWND_TOPBORDER;
    bottomborder_ = UPPSKINWND_BOTTOMBORDER;
    leftborder_ = UPPSKINWND_LEFTBORDER;
    rightborder_ = UPPSKINWND_RIGHTBORDER;
    transmouseing_ = FALSE;
    mouseclientmove_ = FALSE;

    delayrefreshnum_ = 0;

    skinalpha_ = FALSE;
    constalpha_ = 0xFF;

    calc_skin_rgn_ = FALSE;

}

SkinWnd::~SkinWnd() {
}


BOOL SkinWnd::BuildFromXmlFile(const char *xmlfile , bool filebom) {
    String xmldata = filebom ? LoadFileBOM(xmlfile) : LoadFile(xmlfile);
    return BuildFromXml(~xmldata);
}

BOOL SkinWnd::BuildFromSkinLayout(const char *layid) {
    String xmldata = theSkinMgr.ExtractLayoutXml(String(layid));
    return BuildFromXml(~xmldata);
}

BOOL SkinWnd::BuildFromXml(const char *xmlstr) {
    XmlNode xml = ParseXML(xmlstr);
    if(xml.IsEmpty())
        return FALSE;
    if(xml.GetCount() != 1)
        return FALSE;
    const XmlNode &wndNode = xml.At(0);
    if(!wndNode.IsTag("SkinWnd"))
        return FALSE;
    Size szDef(0 , 0);
    Size szMin(0 , 0);
    Size szImg(0 , 0);
    bool _sizeable = false;
    bool _toolwnd = false;
    bool _frameless = false;
    bool _fullscreen = false;

    LargeIcon(theSkinMgr.ExtractLargeIcon("main"));
    Icon(theSkinMgr.ExtractSmallIcon("main"));

    for(int attr_i = 0 ; attr_i < wndNode.GetAttrCount() ; attr_i++) {
        String attrTag = wndNode.AttrId(attr_i);
        String attrVal = wndNode.Attr(attr_i);
        if(attrTag == "layid") {
            LayoutId(attrVal);
        } else if(attrTag == "layidc") {
            LayoutId(attrVal);
        } else if(attrTag == "style") {
            UpdateStyle(attrVal);
            szImg = skinstyle_.GetImageSize();
        }
        else if(attrTag == "defsize") szDef = UIHelper::ParseSize(attrVal);
        else if(attrTag == "minsize") szMin = UIHelper::ParseSize(attrVal);
        else if(attrTag == "sizeable") _sizeable = UIHelper::ParseBool(attrVal) ;
        else if(attrTag == "toolwnd") _toolwnd =  UIHelper::ParseBool(attrVal) ;
        else if(attrTag == "frameless") _frameless = UIHelper::ParseBool(attrVal) ;
        else if(attrTag == "fullscreen") _fullscreen = UIHelper::ParseBool(attrVal) ;
        else if(attrTag == "mouseclientmove") mouseclientmove_ = UIHelper::ParseBool(attrVal) ;
        else if(attrTag == "topborder") topborder_ = (UINT)atoi(attrVal) ;
        else if(attrTag == "bottomborder") bottomborder_ = (UINT)atoi(attrVal) ;
        else if(attrTag == "leftborder") leftborder_ = (UINT)atoi(attrVal) ;
        else if(attrTag == "rightborder") rightborder_ = (UINT)atoi(attrVal) ;
        else if(attrTag == "title") Title(attrVal);
        else if(attrTag == "skinalpha") skinalpha_ = UIHelper::ParseBool(attrVal) ;
        else if(attrTag == "constalpha") constalpha_ = (Upp::byte)(UINT)atoi(attrVal) ;
        else if(attrTag == "calc_skin_rgn") calc_skin_rgn_ = UIHelper::ParseBool(attrVal) ;
    }
    if(szMin.cx < szImg.cx) szMin.cx = szImg.cx;
    if(szMin.cy < szImg.cy) szMin.cy = szImg.cy;
    if(szDef.cx < szMin.cx) szDef.cx = szMin.cx;
    if(szDef.cy < szMin.cy) szDef.cy = szMin.cy;
    Rect rcDef(0 , 0 , 0 , 0);
    if(IsOpen()) {
        rcDef = GetScreenClient(GetHWND());
        if(rcDef.GetWidth() < szDef.cx)
            rcDef.right = rcDef.left + szDef.cx ;
        if(rcDef.GetHeight() < szDef.cy)
            rcDef.bottom = rcDef.top + szDef.cy ;
    }else {
        rcDef.Set(Point(0, 0) , szDef);
    }
    vClippings_.clear();
    for(int child_i = 0 ; child_i < wndNode.GetCount() ; child_i++) {
        if(!CreateChildCtrls(this , wndNode.Node(child_i))) {
            return FALSE;
        }
    }
    Sizeable(_sizeable).ToolWindow(_toolwnd).FrameLess(_frameless);/*.FullScreen( _fullscreen )*/;
    SetMinSize(szMin);
    SetRect(rcDef);

    if(~closeboxbtn_ != NULL)
        *closeboxbtn_ << callback(this , &SkinWnd::OnWindowClose);
    if(~minboxbtn_ != NULL)
        *minboxbtn_ << callback(this , &SkinWnd::OnWindowMinimize);
    if(~maxboxbtn_ != NULL)
        *maxboxbtn_ << callback(this , &SkinWnd::OnWindowMaximize);
    if(~restoreboxbtn_ != NULL)
        *restoreboxbtn_ << callback(this , &SkinWnd::OnWindowMaximize);

    OnBuildFinished();
    //FrameTop
    return TRUE;
}

void SkinWnd::OnBuildFinished() {
    WithDropChoice<EditString> *withdropchoice = dynamic_cast<WithDropChoice<EditString>*>(GetCtrlByLayoutId("userlist"));
    withdropchoice->AddList("ssss");
    withdropchoice->AddList("aaaa");
    withdropchoice->SetLineCy(20);
    withdropchoice->SetFocus();

    EditField* inputname = dynamic_cast<EditField*>(GetCtrlByLayoutId("inputname"));
    inputname->NoBackground(true);

	//CenterWindow(GetHWND());

    //ImageCtrl* image1 = dynamic_cast<ImageCtrl*>(GetCtrlByLayoutId("image1"));
    //image1->SetImage(theSkinMgr.ExtractImage("res/image/search.png"));

    //ImageCtrl* image2 = dynamic_cast<ImageCtrl*>(GetCtrlByLayoutId("image2"));
    //image2->SetImage(theSkinMgr.ExtractImage("res/image/backup_sms.png"));
}

void SkinWnd::OnWindowClose() {
    Close();
}

void SkinWnd::OnWindowMaximize() {
    if(state != MAXIMIZED)
        Maximize(true);
    else
        Overlap(false);
}

void SkinWnd::OnWindowMinimize() {
    Minimize(true);
}


void SkinWnd::WndInvalidateRect(const Rect& r) {
    BaseWnd::WndInvalidateRect(r);
    if(skinalpha_) {
        if(delayrefreshnum_ <= 0) {
            delayrefreshnum_ = 0;
            if(PostMessage(GetHWND() , DELAY_REFRESH_MSG , 0 , 0))
                delayrefreshnum_++;
        }
    }
}

void SkinWnd::WndScrollView(const Rect& r, int dx, int dy) {
    BaseWnd::WndScrollView(r , dx , dy);
    if(skinalpha_) {
        if(delayrefreshnum_ <= 0) {
            delayrefreshnum_ = 0;
            if(PostMessage(GetHWND() , DELAY_REFRESH_MSG , 0 , 0))
                delayrefreshnum_++;
        }
    }
}

void SkinWnd::UpdateLayedWindowShow(BYTE byteConstantAlpha) {
    HWND hwnd = GetHWND();
    if(IsVisible() && !IsIconic(hwnd)) {
        if(IsVisible())
            SyncScroll();
        Rect rc = GetRect();
        fullrefresh = false;
        if(IsVisible()) {
            HDC dc = GetDC(hwnd);
            painting = true;
            ImageDraw imgDraw(rc.GetSize());
            rc.Offset(-rc.TopLeft().x , -rc.TopLeft().y);
            UpdateArea(imgDraw, rc);
            painting = false;
            BLENDFUNCTION Blend = {AC_SRC_OVER, 0, byteConstantAlpha, AC_SRC_ALPHA };
            //Blend.BlendOp = AC_SRC_OVER;
            //Blend.BlendFlags = 0;
            //Blend.AlphaFormat = AC_SRC_ALPHA;
            //Blend.SourceConstantAlpha = byteConstantAlpha;
            CRect rcWnd;
            GetWindowRect(hwnd , & rcWnd);
            ::UpdateLayeredWindow(hwnd, dc,
                                  &CPoint(rcWnd.left, rcWnd.top),
                                  &CSize(rcWnd.right - rcWnd.left , rcWnd.bottom - rcWnd.top),
                                  imgDraw.rgb.dc, &CPoint(0, 0), 0, &Blend, ULW_ALPHA);
            ReleaseDC(hwnd , dc);
        }
    }
}

LRESULT SkinWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT lRet = 0L;
    BOOL bHandled = FALSE;
    switch(message) {
    case WM_CREATE:
        bHandled = TRUE;
        lRet = OnCreate(message , wParam , lParam , bHandled);
        break;
    case WM_NCPAINT:
        bHandled = TRUE;
        lRet = OnNcPaint(message , wParam , lParam , bHandled);
        break;
    case WM_NCCALCSIZE:
        bHandled = TRUE;
        lRet = OnNcCalcSize(message , wParam , lParam , bHandled);
        break;
    case WM_NCACTIVATE:
        bHandled = TRUE;
        lRet = OnNCActivate(message , wParam , lParam , bHandled);
        break;
    case WM_NCHITTEST:
        bHandled = TRUE;
        lRet = OnNcHitTest(message , wParam , lParam , bHandled);
        break;
    case WM_SETCURSOR:
        bHandled = TRUE;
        lRet = OnSetCursor(message , wParam , lParam , bHandled);
        break;
        //case WM_INITMENUPOPUP:
        //  bHandled = TRUE;
        //  lRet = OnInitMenuPopup( message , wParam , lParam , bHandled );
        //  break;
    case WM_WINDOWPOSCHANGED:
        bHandled = TRUE;
        lRet = OnWindowPosChanged(message , wParam , lParam , bHandled);
        break;
    case WM_SIZE:
        bHandled = TRUE;
        lRet = OnSize(message , wParam , lParam , bHandled);
        break;
    case DELAY_REFRESH_MSG:
        bHandled = TRUE;
        lRet = OnDelayRefreshWindow(message , wParam , lParam , bHandled);
        break;
    case WM_NCMOUSEMOVE:
    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONUP:
    case WM_NCLBUTTONDBLCLK:
    case WM_NCRBUTTONDOWN:
    case WM_NCRBUTTONUP:
    case WM_NCRBUTTONDBLCLK:
    case WM_NCMBUTTONDOWN:
    case WM_NCMBUTTONUP:
    case WM_NCMBUTTONDBLCLK:
        bHandled = TRUE;
        lRet = OnNcMouseEvent0(message , wParam , lParam , bHandled);
        break;
    case WM_NCMOUSEHOVER:
    case WM_NCMOUSELEAVE:
        bHandled = TRUE;
        lRet = OnNcMouseEvent1(message , wParam , lParam , bHandled);
        break;

    default:
        break;
    }
    if(!bHandled)
        lRet = BaseWnd::WindowProc(message, wParam, lParam);
    if(message == WM_CREATE) {
        OnCreateFinished(message, wParam, lParam);
    }
    return lRet;
}

LRESULT SkinWnd::OnSize(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    LRESULT lRet = BaseWnd::WindowProc(message, wParam, lParam);
    HWND hWnd = GetHWND();
    if(hWnd != NULL && !IsIconic(hWnd)) {
        HRGN hRgn = NULL;
        Rect rc = GetRect();
        rc.Offset(-rc.left , -rc.top);
        if(calc_skin_rgn_) {
            ImageDraw iw(rc.Width()  , rc.Height());
            DrawBackground(iw.Alpha());
            DrawBackground(iw);
            UIHelper::GetImageDrawClipRegionData(iw.Get(false) , &vClippings_);
        }
        if(!vClippings_.empty()) {
            RECT rcc = { 0 , 0 , rc.Width() , rc.Height() };
            hRgn = UIHelper::CalcClipRegion(rc , &vClippings_);
        }
        if(hRgn) {
            SetWindowRgn(hWnd , hRgn , FALSE);
            DeleteObject(hRgn);
        }
    }
    return lRet;
}

LRESULT SkinWnd::OnWindowPosChanged(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    LRESULT lRet = BaseWnd::WindowProc(message, wParam, lParam);
    WINDOWPLACEMENT wp = {0};
    wp.length = sizeof(WINDOWINFO);
    ::GetWindowPlacement(GetHWND(), &wp);
    bool bMax = (wp.showCmd == SW_SHOWMAXIMIZED);
    if(~maxboxbtn_ != NULL)
        maxboxbtn_->Show(!bMax);
    if(~restoreboxbtn_ != NULL)
        restoreboxbtn_->Show(bMax);
    return lRet;
}

LRESULT SkinWnd::OnDelayRefreshWindow(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    if(--delayrefreshnum_ <= 0) {
        delayrefreshnum_ = 0;
        UpdateLayedWindowShow(constalpha_);
    }
    return 1L;
}

LRESULT SkinWnd::OnInitMenuPopup(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    try {
        HMENU menu = (HMENU)wParam;
        UINT pos = GET_X_LPARAM(lParam);
        BOOL bSys = GET_Y_LPARAM(lParam);
        if(bSys) {
            if(EnableMenuItem(menu , SC_SIZE , MF_ENABLED | MF_BYCOMMAND)) {
            }
        }
    }catch(...) {
    }
    return 0L;
    bHandled = FALSE;
    return 1L;
}

Upp::Image SkinWnd::CursorImage(Upp::Point p, Upp::dword keyflags) {
    POINT pt = { p.x , p.y };
    DWORD dwHitTest = (DWORD)HitTest(pt);
    Image imageCur;
    if(dwHitTest == HTCAPTION || dwHitTest == HTSYSMENU ||
            dwHitTest == HTMENU /*|| dwHitTest == HTCLIENT*/) {
        imageCur = Image::Arrow();
    }else if(dwHitTest == HTTOP || dwHitTest == HTBOTTOM) {
        imageCur = Image::SizeVert();
    }else if(dwHitTest == HTLEFT || dwHitTest == HTRIGHT) {
        imageCur = Image::SizeHorz();
    }else if(dwHitTest == HTTOPLEFT || dwHitTest == HTBOTTOMRIGHT) {
        imageCur = Image::SizeTopLeft();
    }else if(dwHitTest == HTTOPRIGHT || dwHitTest == HTBOTTOMLEFT) {
        imageCur = Image::SizeTopRight();
    }
    return imageCur;
}

LRESULT SkinWnd::OnSetCursor(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    HWND hWnd = GetHWND();
    if(hWnd) {
        if(hCursor && (HWND)wParam == hWnd) {
            SetCursor(hCursor);
        }else {
            return DefWindowProc(hWnd , message , wParam , lParam)    ;
        }
    }else {
        bHandled = FALSE;
    }
    return 1L;
}

LRESULT SkinWnd::OnCreate(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    if(skinalpha_ || constalpha_ != 0xFF) {
        CWindow(GetHWND()).ModifyStyleEx(0 , WS_EX_LAYERED , 0);
        if(skinalpha_)
            UpdateLayedWindowShow(constalpha_);
        else
            SetAlpha(constalpha_);
    }
    bHandled = FALSE;
    return 1L;
}

LRESULT SkinWnd::OnGetMinMaxInfo(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    bHandled = FALSE;
    return 0;
}

LRESULT SkinWnd::OnNCActivate(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    Refresh();
    return 1L;
}

LRESULT SkinWnd::OnActivate(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    return 0L;
}


LRESULT SkinWnd::OnNcPaint(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    return 0L;
}

LRESULT SkinWnd::OnNcCalcSize(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    return 0L;
}

LRESULT SkinWnd::OnNcHitTest(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    HWND hWnd = GetHWND();
    POINT pt = { GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
    ScreenToClient(hWnd , &pt);
    return HitTest(pt);
}

LRESULT SkinWnd::OnNcMouseEvent0(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    HWND hWnd = GetHWND();
    LRESULT lRet = 0;
    POINT point = {0};
    GetCursorPos(&point);
    if(message == WM_NCLBUTTONDOWN || message == WM_NCRBUTTONDOWN || message == WM_NCMBUTTONDOWN) {
        lRet = BaseWnd::WindowProc(message, wParam, lParam);
    } else {
        BOOL bDef = FALSE;
        if(message == WM_NCLBUTTONDBLCLK) {
            if(~titlebar_ != NULL && HTCAPTION == wParam && maximizebox) {
                POINT pt = point;
                ScreenToClient(hWnd , &pt);
                if(titlebar_->GetRect().Contains(pt)) {
                    bDef = TRUE;
                }
            }
        } else {
            bDef = TRUE;
        }
        if(bDef)
            lRet = DefWindowProc(hWnd , message, wParam, lParam);
    }
    if(wParam != HTCLIENT && wParam != HTCAPTION && wParam != HTTRANSPARENT) {
        if(message == WM_NCLBUTTONDOWN && IsSizeable()) {
            if(wParam == HTTOP)
                SendMessage(hWnd , WM_SYSCOMMAND, SC_SIZE | WMSZ_TOP, MAKELPARAM(point.x, point.y));
            else if(wParam == HTBOTTOM)
                SendMessage(hWnd , WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOM, MAKELPARAM(point.x, point.y));
            else if(wParam == HTLEFT)
                SendMessage(hWnd , WM_SYSCOMMAND, SC_SIZE | WMSZ_LEFT, MAKELPARAM(point.x, point.y));
            else if(wParam == HTRIGHT)
                SendMessage(hWnd , WM_SYSCOMMAND, SC_SIZE | WMSZ_RIGHT, MAKELPARAM(point.x, point.y));
            else if(wParam == HTTOPLEFT)
                SendMessage(hWnd , WM_SYSCOMMAND, SC_SIZE | WMSZ_TOPLEFT, MAKELPARAM(point.x, point.y));
            else if(wParam == HTTOPRIGHT)
                SendMessage(hWnd , WM_SYSCOMMAND, SC_SIZE | WMSZ_TOPRIGHT, MAKELPARAM(point.x, point.y));
            else if(wParam == HTBOTTOMLEFT)
                SendMessage(hWnd , WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOMLEFT, MAKELPARAM(point.x, point.y));
            else if(wParam == HTBOTTOMRIGHT)
                SendMessage(hWnd , WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOMRIGHT, MAKELPARAM(point.x, point.y));
        }
    }
    if(TRUE) {
        ScreenToClient(hWnd , &point);
        WORD dwKeyState = 0;
        if(GetCtrl()) dwKeyState |= MK_CONTROL;
        if(GetShift()) dwKeyState |= MK_SHIFT;
        if(GetMouseLeft()) dwKeyState |= MK_LBUTTON;
        if(GetMouseRight()) dwKeyState |= MK_RBUTTON;
        if(GetMouseMiddle()) dwKeyState |= MK_MBUTTON;
        if(!!(GetKeyState(VK_XBUTTON1) & 0x8000)) dwKeyState |= MK_XBUTTON1;
        if(!!(GetKeyState(VK_XBUTTON2) & 0x8000)) dwKeyState |= MK_XBUTTON2;
        transmouseing_ = TRUE;
        SendMessage(hWnd , WM_MOUSEMOVE - WM_NCMOUSEMOVE + message , MAKELPARAM(dwKeyState , 0) , MAKELPARAM(point.x , point.y));
        transmouseing_ = FALSE;
        //if(  HasMouseDeep() && GetMouseCtrl() != this )
        //  return lRet;
    }
    return lRet;
}

LRESULT SkinWnd::OnNcMouseEvent1(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    HWND hWnd = GetHWND();
    bHandled = FALSE;
    POINT pt = { 0 , 0 };
    GetCursorPos(&pt);
    ScreenToClient(hWnd , &pt);
    SendMessage(hWnd , message == WM_NCMOUSEHOVER ? WM_MOUSEHOVER : WM_MOUSELEAVE , wParam , MAKELPARAM(pt.x , pt.y));
    return 0L;
}

LRESULT SkinWnd::OnCreateFinished(UINT message, WPARAM wParam, LPARAM lParam) {
    SyncCaption();
    SyncTitle();
	CWindow wnd = GetHWND();
	wnd.CenterWindow();

/* fullscreen app?
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	CPoint ptLeftTop(1, 1), ptRightTop(screenWidth - 1, 1);
	CPoint ptLeftBottom(1, screenHeight - 1), ptRightBottom(1, screenHeight - 1);
	CWindow wndLT = WindowFromPoint(ptLeftTop);
	CWindow wndRT = WindowFromPoint(ptRightTop);
	CWindow wndLB = WindowFromPoint(ptLeftBottom);
	CWindow wndRB = WindowFromPoint(ptRightBottom);
*/

    return 1L;
}

void SkinWnd::Paint(Draw& w) {
    if(IsIconic(GetHWND()))
        return;
    DrawBackground(w);
}

void SkinWnd::SyncCaption() {
    GuiLock __;
    LLOG("SkinWnd::SyncCaption");
    if(fullscreen)
        return;
    HWND hwnd = GetHWND();
    if(!IsWindow(hwnd)) {
        return;
    }
    if(hwnd) {
        style = ::GetWindowLong(hwnd, GWL_STYLE);
        exstyle = ::GetWindowLong(hwnd, GWL_EXSTYLE);
    }
    style &= ~(WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_DLGFRAME);
    exstyle &= ~(WS_EX_TOOLWINDOW | WS_EX_DLGMODALFRAME);
    style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU /*| WS_THICKFRAME */;
    if(minimizebox) {
        style |= WS_MINIMIZEBOX;
    }
    if(maximizebox) {
        style |= WS_MAXIMIZEBOX;
    }
    if(frameless) {
        style = (style & ~WS_CAPTION) ;
    }else if(IsNull(icon) && !maximizebox && !minimizebox || noclosebox) {
        style |= WS_POPUPWINDOW | WS_DLGFRAME;
        exstyle |= WS_EX_DLGMODALFRAME;
        if(noclosebox)
            style &= ~WS_SYSMENU;
    }else {
        style |= WS_SYSMENU;
    }
    if(tool) {
        exstyle |= WS_EX_TOOLWINDOW;
    }
    if(fullscreen) {
        style = WS_POPUP;
    }
    if(hwnd) {
        ::SetWindowLong(hwnd, GWL_STYLE, style);
        ::SetWindowLong(hwnd, GWL_EXSTYLE, exstyle);
        SyncTitle();
        if(urgent) {
            if(IsForeground())
                urgent = false;
            FLASHWINFO fi;
            memset(&fi, 0, sizeof(fi));
            fi.cbSize = sizeof(fi);
            fi.hwnd = hwnd;
            fi.dwFlags = urgent ? FLASHW_TIMER | FLASHW_ALL : FLASHW_STOP;
            FlashWindowEx(&fi);
        }
    }
    DeleteIco();
    if(hwnd) {
        ::SendMessage(hwnd, WM_SETICON, false, (LPARAM)(ico = IconWin32(icon)));
        ::SendMessage(hwnd, WM_SETICON, true, (LPARAM)(lico = IconWin32(largeicon)));
    }
}


void SkinWnd::DrawBackground(Upp::Draw& w) {
    Rect rc = GetRect();
    rc.Offset(-rc.left , -rc.top);
    if(skinstyle_.bkgnd_type_ == SkinWnd::Style::FIXED)
        w.DrawImage(rc.left , rc.top , rc.GetWidth() , rc.GetHeight() , skinstyle_.image_[0]);
    else if(skinstyle_.bkgnd_type_ == SkinWnd::Style::GRID)
        UIHelper::DrawGridImage(w , Rect(rc.TopLeft() , rc.GetSize()) , skinstyle_.image_);
    else if(skinstyle_.bkgnd_type_ == SkinWnd::Style::HORZ3)
        UIHelper::DrawHorz3Image(w , Rect(rc.TopLeft() , rc.GetSize()) , skinstyle_.image_);
    else if(skinstyle_.bkgnd_type_ == SkinWnd::Style::VERT3)
        UIHelper::DrawVert3Image(w , Rect(rc.TopLeft() , rc.GetSize()) , skinstyle_.image_);
    return;
}

LRESULT SkinWnd::HitTest(const POINT &pt) {
    HWND hWnd = GetHWND();
    RECT rc = {0};
    GetClientRect(hWnd , &rc);
    LRESULT lRet = HTCLIENT;
    if(IsSizeable()) {
        if(pt.x < leftborder_ && pt.y < topborder_)
            lRet = HTTOPLEFT;
        else if(pt.x < leftborder_ && pt.y > rc.bottom - bottomborder_)
            lRet = HTBOTTOMLEFT;
        else if(pt.x > rc.right - rightborder_ && pt.y < topborder_)
            lRet = HTTOPRIGHT;
        else if(pt.x > rc.right - rightborder_ && pt.y > rc.bottom - bottomborder_)
            lRet = HTBOTTOMRIGHT;
        else if(pt.x < leftborder_)
            lRet = HTLEFT;
        else if(pt.x > rc.right - rightborder_)
            lRet = HTRIGHT;
        else if(pt.y < topborder_)
            lRet = HTTOP;
        else if(pt.y > rc.bottom - bottomborder_)
            lRet = HTBOTTOM;
        else if(~sizeboxbtn_ != NULL) {
            if(sizeboxbtn_->GetRect().Contains(pt.x , pt.y))
                lRet = HTBOTTOMRIGHT;
        }
    }
    Ctrl *mouseCtrl = GetMouseCtrl();
    if(~titlebar_ != NULL) {
        if(titlebar_->GetRect().Contains(pt.x , pt.y)) {
            if(mouseCtrl != NULL && HasChildDeep(mouseCtrl) 
                    && (mouseCtrl == this || mouseCtrl->GetHitTest(pt) == HTTRANSPARENT/*dynamic_cast<ParentCtrl*>(mouseCtrl) != NULL*/)) {
                if(lRet == HTCLIENT)
                    lRet = HTCAPTION;
            }
        }
    }
    if(mouseCtrl != NULL && HasChildDeep(mouseCtrl) 
            && (mouseCtrl == this || mouseCtrl->GetHitTest(pt) == HTTRANSPARENT/*dynamic_cast<ParentCtrl*>(mouseCtrl) != NULL)*/)) {
        if(mouseclientmove_ && lRet == HTCLIENT) {
            lRet = HTCAPTION ;
        }
    }else {
        lRet = HTCLIENT;
    }
    
    return lRet;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const SkinWnd::Style& SkinWnd::GetStyle() {
    return skinstyle_;
}

void SkinWnd::SetStyle(const SkinWnd::Style& style) {
    skinstyle_ = style;
}

const SkinWnd::Style SkinWnd::StyleNormal() {
    SkinWnd::Style style;
    RGBA rgba;
    rgba.r = rgba.g = rgba.b = 0x00;
    rgba.a = 0xFF;
    Image img = UIHelper::CreateMonoImage(rgba , Size(1, 1));
    for(int i = 0 ; i < 9 ; i++)
        style.image_[i] = img;
    style.bkgnd_type_ = SkinWnd::Style::GRID;
    return style;
}


Size SkinWnd::Style::GetImageSize() {
    Size sz(0 , 0);
    if(bkgnd_type_ == SkinWnd::Style::FIXED) {
        sz.cx = image_[0].GetWidth();
        sz.cy = image_[0].GetHeight();
    }else if(bkgnd_type_ == SkinWnd::Style::HORZ3) {
        sz.cx = image_[0].GetWidth();
        sz.cy = image_[0].GetHeight()
                + image_[1].GetHeight()
                + image_[2].GetHeight();
    }else if(bkgnd_type_ == SkinWnd::Style::VERT3) {
        sz.cx = image_[0].GetWidth()
                + image_[1].GetWidth()
                + image_[2].GetWidth();
        sz.cy = image_[0].GetHeight();
    }else if(bkgnd_type_ == SkinWnd::Style::GRID) {
        sz.cx = image_[0].GetWidth()
                + image_[1].GetWidth()
                + image_[2].GetWidth();
        sz.cy = image_[0].GetHeight()
                + image_[3].GetHeight()
                + image_[6].GetHeight();
    }
    return sz;
}

void SkinWnd::UpdateStyle(String attrVal) {
    if(attrVal == "wnd_background_grid") {
        SkinWnd::Style uppskinwnd_grid;
        uppskinwnd_grid.bkgnd_type_ = SkinWnd::Style::GRID;
        uppskinwnd_grid.image_[0] = theSkinMgr.ExtractImage("wnd_topleft");
        uppskinwnd_grid.image_[1] = theSkinMgr.ExtractImage("wnd_topmiddle");
        uppskinwnd_grid.image_[2] = theSkinMgr.ExtractImage("wnd_topright");
        uppskinwnd_grid.image_[3] = theSkinMgr.ExtractImage("wnd_centerleft");
        uppskinwnd_grid.image_[4] = theSkinMgr.ExtractImage("wnd_centermiddle");
        uppskinwnd_grid.image_[5] = theSkinMgr.ExtractImage("wnd_centerright");
        uppskinwnd_grid.image_[6] = theSkinMgr.ExtractImage("wnd_bottomleft");
        uppskinwnd_grid.image_[7] = theSkinMgr.ExtractImage("wnd_bottommiddle");
        uppskinwnd_grid.image_[8] = theSkinMgr.ExtractImage("wnd_bottomright");
        SetStyle(uppskinwnd_grid);   
    } else if(attrVal == "wnd_background_color_grid") {
        SkinWnd::Style uppskinwnd_color_grid;
        uppskinwnd_color_grid.bkgnd_type_ = SkinWnd::Style::GRID;
        uppskinwnd_color_grid.image_[0] = theSkinMgr.ExtractImage("wnd_color_topleft");
        uppskinwnd_color_grid.image_[1] = theSkinMgr.ExtractImage("wnd_color_topmiddle");
        uppskinwnd_color_grid.image_[2] = theSkinMgr.ExtractImage("wnd_color_topright");
        uppskinwnd_color_grid.image_[3] = theSkinMgr.ExtractImage("wnd_color_centerleft");
        uppskinwnd_color_grid.image_[4] = theSkinMgr.ExtractImage("wnd_color_centermiddle");
        uppskinwnd_color_grid.image_[5] = theSkinMgr.ExtractImage("wnd_color_centerright");
        uppskinwnd_color_grid.image_[6] = theSkinMgr.ExtractImage("wnd_color_bottomleft");
        uppskinwnd_color_grid.image_[7] = theSkinMgr.ExtractImage("wnd_color_bottommiddle");
        uppskinwnd_color_grid.image_[8] = theSkinMgr.ExtractImage("wnd_color_bottomright");
        SetStyle(uppskinwnd_color_grid); 
    } else {
        assert(false);
        SetStyle(StyleNormal());
    }
}

void SkinWnd::UpdateSysButtonStyle(Upp::Button* button, Upp::String attrVal) {
    if(attrVal == "sys_sizebtn") {
        std::tr1::shared_ptr<Upp::Button::Style> sizebox_style(new(std::nothrow) Button::Style());
        sysbtn_styles_.push_back(sizebox_style);
        sizebox_style->Assign(Button::StyleNormal());
        sizebox_style->look[0] = theSkinMgr.ExtractImage("sys_sizebox");
        sizebox_style->look[1] = theSkinMgr.ExtractImage("sys_sizebox");
        sizebox_style->look[2] = theSkinMgr.ExtractImage("sys_sizebox");
        sizebox_style->look[3] = theSkinMgr.ExtractImage("sys_sizebox");
        button->SetStyle(*sizebox_style);
        
    } else if(attrVal == "sys_closebtn") {
        std::tr1::shared_ptr<Upp::Button::Style> sys_close_style(new(std::nothrow) Button::Style());
        sysbtn_styles_.push_back(sys_close_style);
        sys_close_style->Assign(Button::StyleNormal());
        sys_close_style->look[0] = theSkinMgr.ExtractImage("sys_close_normal");
        sys_close_style->look[1] = theSkinMgr.ExtractImage("sys_close_hover");
        sys_close_style->look[2] = theSkinMgr.ExtractImage("sys_close_down");
        sys_close_style->look[3] = theSkinMgr.ExtractImage("sys_close_disable");
        button->SetStyle(*sys_close_style);
    } else if(attrVal == "sys_minbtn") {
        std::tr1::shared_ptr<Upp::Button::Style> sys_min_style(new(std::nothrow) Button::Style());
        sysbtn_styles_.push_back(sys_min_style);
        sys_min_style->Assign(Button::StyleNormal());
        sys_min_style->look[0] = theSkinMgr.ExtractImage("sys_min_normal");
        sys_min_style->look[1] = theSkinMgr.ExtractImage("sys_min_hover");
        sys_min_style->look[2] = theSkinMgr.ExtractImage("sys_min_down");
        sys_min_style->look[3] = theSkinMgr.ExtractImage("sys_min_disable");
        button->SetStyle(*sys_min_style);
    } else if(attrVal == "sys_maxbtn") {
        std::tr1::shared_ptr<Upp::Button::Style> sys_max_style(new(std::nothrow) Button::Style());
        sysbtn_styles_.push_back(sys_max_style);
        sys_max_style->Assign(Button::StyleNormal());
        sys_max_style->look[0] = theSkinMgr.ExtractImage("sys_max_normal");
        sys_max_style->look[1] = theSkinMgr.ExtractImage("sys_max_hover");
        sys_max_style->look[2] = theSkinMgr.ExtractImage("sys_max_down");
        sys_max_style->look[3] = theSkinMgr.ExtractImage("sys_max_disable");
        button->SetStyle(*sys_max_style);
    } else if(attrVal == "sys_restorebtn") {
        std::tr1::shared_ptr<Upp::Button::Style> sys_restore_style(new(std::nothrow) Button::Style());
        sysbtn_styles_.push_back(sys_restore_style);
        sys_restore_style->Assign(Button::StyleNormal());
        sys_restore_style->look[0] = theSkinMgr.ExtractImage("sys_restore_normal");
        sys_restore_style->look[1] = theSkinMgr.ExtractImage("sys_restore_hover");
        sys_restore_style->look[2] = theSkinMgr.ExtractImage("sys_restore_down");
        sys_restore_style->look[3] = theSkinMgr.ExtractImage("sys_restore_disable");
        button->SetStyle(*sys_restore_style);
    }
}



////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void Common::Util::SetWindowIconTitle(HWND hwnd, UINT iconID, LPCTSTR title) {
    CWindow wnd(hwnd);
    if (!wnd.IsWindow()) {
        return;
    }
    HICON hIconBig = AtlLoadIconImage(iconID, LR_DEFAULTCOLOR | LR_SHARED, 
        GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
    wnd.SetIcon(hIconBig, TRUE);
    HICON hIconSmall = AtlLoadIconImage(iconID, LR_DEFAULTCOLOR | LR_SHARED, 
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CXSMICON));
    wnd.SetIcon(hIconSmall, FALSE);
    wnd.SetWindowText(title);
}
*/

END_UPPEX_NAMESPACE