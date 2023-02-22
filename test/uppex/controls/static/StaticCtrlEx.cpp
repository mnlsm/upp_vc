#include "stdafx.h"
#include "StaticCtrlEx.h"
using namespace Upp;
NAMESPACE_UPPEX

StaticCtrlEx::StaticCtrlEx() {
    hittest = HTTRANSPARENT;
}

StaticCtrlEx::~StaticCtrlEx() {
}

void StaticCtrlEx::Paint(Upp::Draw& w) {
    Label::Paint(w);
}


BOOL StaticCtrlEx::SetAttrParam(BOOL bSetPrev, Upp::String attrId, Upp::String attrVal) {
    if(attrId == "text") {
        SetText(attrVal);
    } else if(attrId == "font") {
        lbl.font = theSkinMgr.ExtractFont(attrVal);
    } else if(attrId == "tclrid") {
        lbl.ink = lbl.disabledink = theSkinMgr.ExtractColor(attrVal);
    } else if(attrId == "align") {
        SetAlign(UIHelper::ParseAlignment(attrVal));
    } else if(attrId == "valign") {
        SetVAlign(UIHelper::ParseAlignment(attrVal));
    }

    
    return TRUE;
}



END_UPPEX_NAMESPACE
