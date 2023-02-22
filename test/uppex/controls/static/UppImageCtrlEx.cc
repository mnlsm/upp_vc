#include "stdafx.h"
#include "UppImageCtrlEx.h"

using namespace Upp;
NAMESPACE_UPPEX

UppImageCtrlEx::UppImageCtrlEx() {
    hittest = HTTRANSPARENT;
}

UppImageCtrlEx::~UppImageCtrlEx() {
}

BOOL UppImageCtrlEx::SetAttrParam(BOOL bSetPrev, String attrId, String attrVal) {
    if(attrId == "src") {
        SetImage(theSkinMgr.ExtractImage(attrVal));
    }
    return TRUE;
}

void UppImageCtrlEx::Paint(Upp::Draw& w) {
    Upp::ImageCtrl::Paint(w);
}





END_UPPEX_NAMESPACE