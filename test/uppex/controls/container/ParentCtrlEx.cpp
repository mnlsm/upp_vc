#include "stdafx.h"
#include "ParentCtrlEx.h"
#include "../../window/SkinWnd.h"


using namespace Upp;

BEGIN_NAMESPACE_UPPEX

ParentCtrlEx::ParentCtrlEx() {
    hittest = HTTRANSPARENT;
}

ParentCtrlEx::~ParentCtrlEx() {

}


Upp::Image ParentCtrlEx::CursorImage(Upp::Point p, Upp::dword keyflags) {
    Ctrl *top = GetTopCtrl();
    if(top) {
        if(dynamic_cast<SkinWnd*>(top) != NULL) {
            SkinWnd *wnd = (SkinWnd *)(top);
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

END_UPPEX_NAMESPACE