#include "stdafx.h"
#include "ParentCtrlEx.h"
#include "UppSkinWnd.h"


using namespace Upp;
Upp::Image ParentCtrlEx::CursorImage(Upp::Point p, Upp::dword keyflags) {
    Ctrl *top = GetTopCtrl();
    if(top) {
        if(dynamic_cast<CUppSkinWnd*>(top) != NULL) {
            CUppSkinWnd *wnd = (CUppSkinWnd *)(top);
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(wnd->GetHWND() , &pt);
            Image imageCur = wnd->CursorImage(Point(pt.x , pt.y) , keyflags);
            if(!imageCur.IsEmpty())
                return imageCur;
        }
    }
    return ParentCtrl::CursorImage(p , keyflags);
}

