#include "stdafx.h"
#include "UppSkinWnd.h"
#include "UppSkinMgr.h"

using namespace Upp;

#define UPPSKINWND_TOPBORDER    4
#define UPPSKINWND_BOTTOMBORDER 4
#define UPPSKINWND_LEFTBORDER   4
#define UPPSKINWND_RIGHTBORDER  4

#define LLOG(x)
//

#define DELAY_REFRESH_MSG WM_USER + 888



CUppSkinWnd::CUppSkinWnd()
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

CUppSkinWnd::~CUppSkinWnd() {

}


/*
<SkinWnd layid='mainwnd' skin='wnd_background_grid' wndsize='200,100' minsize='80,20' sizeable='false'
         toolwnd='false' frameless='false' fullscreen='false' mouseclientmove='false'
         topborder='4' bottomborder='4' leftborder='4' rightborder='4'>
<ParentCtrl layid='sys_titlebar' HSizePos='5,5' TopPos='0,27'>
    <Button layid='sys_close_btn' tip='关闭' skin='sys_close_btn' RightPos='1,39' TopPos='1,25'/>
    <Button layid='sys_max_btn' tip='最大化' skin='sys_max_btn' RightPos='40,31' TopPos='1,25'/>
    <Button layid='sys_min_btn' tip='最小化' skin='sys_min_btn' RightPos='72,31' TopPos='1,25'/>
</ParentCtrl>
<Button layid='sys_sizebtn' skin='sys_sizebtn' RightPos='1,4' BottomPos='1,4'>
</skinwnd>
*/
BOOL CUppSkinWnd::BuildFromXml(const char *xmlstr) {
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
    for(int attr_i = 0 ; attr_i < wndNode.GetAttrCount() ; attr_i++) {
        String attrTag = wndNode.AttrId(attr_i);
        String attrVal = wndNode.Attr(attr_i);
        if(attrTag == "layid") LayoutId(attrVal);
        else if(attrTag == "skin") {
            SetStyle(theSkinMgr.GetUppSkinWndStyle(attrVal));
            szImg = skinstyle_.GetImageSize();
        }else if(attrTag == "defsize") szDef = UppUIHelper::ParseSize(attrVal);
        else if(attrTag == "minsize") szMin = UppUIHelper::ParseSize(attrVal);
        else if(attrTag == "sizeable") _sizeable = UppUIHelper::ParseBool(attrVal) ;
        else if(attrTag == "toolwnd") _toolwnd =  UppUIHelper::ParseBool(attrVal) ;
        else if(attrTag == "frameless") _frameless = UppUIHelper::ParseBool(attrVal) ;
        else if(attrTag == "fullscreen") _fullscreen = UppUIHelper::ParseBool(attrVal) ;
        else if(attrTag == "mouseclientmove") mouseclientmove_ = UppUIHelper::ParseBool(attrVal) ;
        else if(attrTag == "topborder") topborder_ = (UINT)atoi(attrVal) ;
        else if(attrTag == "bottomborder") bottomborder_ = (UINT)atoi(attrVal) ;
        else if(attrTag == "leftborder") leftborder_ = (UINT)atoi(attrVal) ;
        else if(attrTag == "rightborder") rightborder_ = (UINT)atoi(attrVal) ;
        else if(attrTag == "title") Title(attrVal);
        else if(attrTag == "skinalpha") skinalpha_ = UppUIHelper::ParseBool(attrVal) ;
        else if(attrTag == "constalpha") constalpha_ = (Upp::byte)(UINT)atoi(attrVal) ;
        else if(attrTag == "calc_skin_rgn") calc_skin_rgn_ = UppUIHelper::ParseBool(attrVal) ;
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
        if(!CreateChildCtrls(this , wndNode.Node(child_i)))
            return FALSE;
    }
    Sizeable(_sizeable).ToolWindow(_toolwnd).FrameLess(_frameless);/*.FullScreen( _fullscreen )*/;
    SetMinSize(szMin);
    SetRect(rcDef);

    if(~closeboxbtn_ != NULL)
        *closeboxbtn_ << callback(this , &CUppSkinWnd::OnWindowClose);
    if(~minboxbtn_ != NULL)
        *minboxbtn_ << callback(this , &CUppSkinWnd::OnWindowMinimize);
    if(~maxboxbtn_ != NULL)
        *maxboxbtn_ << callback(this , &CUppSkinWnd::OnWindowMaximize);
    if(~restoreboxbtn_ != NULL)
        *restoreboxbtn_ << callback(this , &CUppSkinWnd::OnWindowMaximize);

    return TRUE;
}

void CUppSkinWnd::OnWindowClose() {
    Close();
}

void CUppSkinWnd::OnWindowMaximize() {
    if(state != MAXIMIZED)
        Maximize(true);
    else
        Overlap(false);
}

void CUppSkinWnd::OnWindowMinimize() {
    Minimize(true);
}


BOOL CUppSkinWnd::BuildFromXmlFile(const char *xmlfile , bool filebom) {
    String xmldata = filebom ? LoadFileBOM(xmlfile) : LoadFile(xmlfile);
    return BuildFromXml(~xmldata);
}


void CUppSkinWnd::WndInvalidateRect(const Rect& r) {
    BaseWnd::WndInvalidateRect(r);
    if(skinalpha_) {
        if(delayrefreshnum_ <= 0) {
            delayrefreshnum_ = 0;
            if(PostMessage(GetHWND() , DELAY_REFRESH_MSG , 0 , 0))
                delayrefreshnum_++;
        }
    }
}

void CUppSkinWnd::WndScrollView(const Rect& r, int dx, int dy) {
    BaseWnd::WndScrollView(r , dx , dy);
    if(skinalpha_) {
        if(delayrefreshnum_ <= 0) {
            delayrefreshnum_ = 0;
            if(PostMessage(GetHWND() , DELAY_REFRESH_MSG , 0 , 0))
                delayrefreshnum_++;
        }
    }
}

void CUppSkinWnd::UpdateLayedWindowShow(BYTE byteConstantAlpha) {
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
            BLENDFUNCTION Blend = {0};
            Blend.BlendOp = AC_SRC_OVER;
            Blend.BlendFlags = 0;
            Blend.AlphaFormat = AC_SRC_ALPHA;
            Blend.SourceConstantAlpha = byteConstantAlpha;
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

LRESULT CUppSkinWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
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
    return lRet;
}

LRESULT CUppSkinWnd::OnSize(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
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
            UppUIHelper::GetImageDrawClipRegionData(iw.Get(false) , &vClippings_);
        }
        if(!vClippings_.empty()) {
            RECT rcc = { 0 , 0 , rc.Width() , rc.Height() };
            hRgn = UppUIHelper::CalcClipRegion(rc , &vClippings_);
        }
        if(hRgn) {
            SetWindowRgn(hWnd , hRgn , FALSE);
            DeleteObject(hRgn);
        }
    }
    return lRet;
}

LRESULT CUppSkinWnd::OnWindowPosChanged(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
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

LRESULT CUppSkinWnd::OnDelayRefreshWindow(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    if(--delayrefreshnum_ <= 0) {
        delayrefreshnum_ = 0;
        UpdateLayedWindowShow(constalpha_);
    }
    return 1L;
}

LRESULT CUppSkinWnd::OnInitMenuPopup(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
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

Upp::Image CUppSkinWnd::CursorImage(Upp::Point p, Upp::dword keyflags) {
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

LRESULT CUppSkinWnd::OnSetCursor(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
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

LRESULT CUppSkinWnd::OnCreate(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
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

LRESULT CUppSkinWnd::OnGetMinMaxInfo(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    bHandled = FALSE;
    return 0;
}

LRESULT CUppSkinWnd::OnNCActivate(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    Refresh();
    return 1L;
}

LRESULT CUppSkinWnd::OnActivate(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    return 0L;
}


LRESULT CUppSkinWnd::OnNcPaint(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    return 0L;
}

LRESULT CUppSkinWnd::OnNcCalcSize(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    return 0L;
}

LRESULT CUppSkinWnd::OnNcHitTest(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    HWND hWnd = GetHWND();
    POINT pt = { GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
    ScreenToClient(hWnd , &pt);
    return HitTest(pt);
}

LRESULT CUppSkinWnd::OnNcMouseEvent0(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    HWND hWnd = GetHWND();
    LRESULT lRet = 0;
    POINT point = {0};
    GetCursorPos(&point);
    if(message == WM_NCLBUTTONDOWN || message == WM_NCRBUTTONDOWN || message == WM_NCMBUTTONDOWN) {
        lRet = BaseWnd::WindowProc(message, wParam, lParam);
    }else {
        BOOL bDef = TRUE;
        if(message == WM_NCLBUTTONDBLCLK && ~titlebar_ != NULL && HTCAPTION == wParam && maximizebox) {
            POINT pt = point;
            ScreenToClient(hWnd , &pt);
            if(!titlebar_->GetRect().Contains(pt))
                bDef = FALSE;
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

LRESULT CUppSkinWnd::OnNcMouseEvent1(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled) {
    HWND hWnd = GetHWND();
    bHandled = FALSE;
    POINT pt = { 0 , 0 };
    GetCursorPos(&pt);
    ScreenToClient(hWnd , &pt);
    SendMessage(hWnd , message == WM_NCMOUSEHOVER ? WM_MOUSEHOVER : WM_MOUSELEAVE , wParam , MAKELPARAM(pt.x , pt.y));
    return 0L;
}

void CUppSkinWnd::Paint(Draw& w) {
    if(IsIconic(GetHWND()))
        return;
    DrawBackground(w);
}

void CUppSkinWnd::SyncCaption() {
    GuiLock __;
    LLOG("CUppSkinWnd::SyncCaption");
    if(fullscreen)
        return;
    HWND hwnd = GetHWND();
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


void CUppSkinWnd::DrawBackground(Upp::Draw& w) {
    Rect rc = GetRect();
    rc.Offset(-rc.left , -rc.top);
    if(skinstyle_.bkgnd_type_ == CUppSkinWnd::Style::FIXED)
        w.DrawImage(rc.left , rc.top , rc.GetWidth() , rc.GetHeight() , skinstyle_.image_[0]);
    else if(skinstyle_.bkgnd_type_ == CUppSkinWnd::Style::GRID)
        UppUIHelper::DrawGridImage(w , Rect(rc.TopLeft() , rc.GetSize()) , skinstyle_.image_);
    else if(skinstyle_.bkgnd_type_ == CUppSkinWnd::Style::HORZ3)
        UppUIHelper::DrawHorz3Image(w , Rect(rc.TopLeft() , rc.GetSize()) , skinstyle_.image_);
    else if(skinstyle_.bkgnd_type_ == CUppSkinWnd::Style::VERT3)
        UppUIHelper::DrawVert3Image(w , Rect(rc.TopLeft() , rc.GetSize()) , skinstyle_.image_);
    return;
}

LRESULT CUppSkinWnd::HitTest(const POINT &pt) {
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
    if(~titlebar_ != NULL) {
        if(titlebar_->GetRect().Contains(pt.x , pt.y)) {
            Ctrl *mouseCtrl = GetMouseCtrl();
            if(mouseCtrl != NULL && HasChildDeep(mouseCtrl) && (mouseCtrl == this || dynamic_cast<ParentCtrl*>(mouseCtrl) != NULL)) {
                if(lRet == HTCLIENT)
                    lRet = HTCAPTION;
            }
        }
    }
    Ctrl *mouseCtrl = GetMouseCtrl();
    if(mouseCtrl != NULL && HasChildDeep(mouseCtrl) && (mouseCtrl == this || dynamic_cast<ParentCtrl*>(mouseCtrl) != NULL)) {
        if(mouseclientmove_ && lRet == HTCLIENT)
            lRet = HTCAPTION ;
    }else {
        lRet = HTCLIENT;
    }
    return lRet;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const CUppSkinWnd::Style& CUppSkinWnd::GetStyle() {
    return skinstyle_;
}

void CUppSkinWnd::SetStyle(const CUppSkinWnd::Style& style) {
    skinstyle_ = style;
}

const CUppSkinWnd::Style CUppSkinWnd::StyleNormal() {
    CUppSkinWnd::Style style;
    RGBA rgba;
    rgba.r = rgba.g = rgba.b = 0x00;
    rgba.a = 0xFF;
    Image img = UppUIHelper::CreateMonoImage(rgba , Size(1, 1));
    for(int i = 0 ; i < 9 ; i++)
        style.image_[i] = img;
    style.bkgnd_type_ = CUppSkinWnd::Style::GRID;
    return style;
}

Size CUppSkinWnd::Style::GetImageSize() {
    Size sz(0 , 0);
    if(bkgnd_type_ == CUppSkinWnd::Style::FIXED) {
        sz.cx = image_[0].GetWidth();
        sz.cy = image_[0].GetHeight();
    }else if(bkgnd_type_ == CUppSkinWnd::Style::HORZ3) {
        sz.cx = image_[0].GetWidth();
        sz.cy = image_[0].GetHeight()
                + image_[1].GetHeight()
                + image_[2].GetHeight();
    }else if(bkgnd_type_ == CUppSkinWnd::Style::VERT3) {
        sz.cx = image_[0].GetWidth()
                + image_[1].GetWidth()
                + image_[2].GetWidth();
        sz.cy = image_[0].GetHeight();
    }else if(bkgnd_type_ == CUppSkinWnd::Style::GRID) {
        sz.cx = image_[0].GetWidth()
                + image_[1].GetWidth()
                + image_[2].GetWidth();
        sz.cy = image_[0].GetHeight()
                + image_[3].GetHeight()
                + image_[6].GetHeight();
    }
    return sz;
}