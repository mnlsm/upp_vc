#include "stdafx.h"
#include "UppImageCtrlEx.h"

using namespace Upp;
NAMESPACE_UPPEX

UppImageCtrlEx::UppImageCtrlEx() {
}

UppImageCtrlEx::~UppImageCtrlEx() {
}

BOOL UppImageCtrlEx::SetAttrParam(BOOL bSetPrev, String attrId, String attrVal) {
    if(attrId == "src") {
        SetImage(theSkinMgr.ExtractImage(attrVal));
    }
    return TRUE;
}




END_UPPEX_NAMESPACE