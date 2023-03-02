#include "stdafx.h"
#include "ImageCtrlEx.h"

using namespace Upp;
BEGIN_NAMESPACE_UPPEX

ImageCtrlEx::ImageCtrlEx() {
    hittest = HTTRANSPARENT;
}

ImageCtrlEx::~ImageCtrlEx() {
}

BOOL ImageCtrlEx::SetAttrParam(BOOL bSetPrev, String attrId, String attrVal) {
    if(attrId == "src") {
        SetImage(theSkinMgr.ExtractImage(attrVal));
    }
    return TRUE;
}

void ImageCtrlEx::Paint(Upp::Draw& w) {
    Upp::ImageCtrl::Paint(w);
}





END_UPPEX_NAMESPACE