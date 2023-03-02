#include "stdafx.h"
#include "ParentCtrlStub.h"
#include "../../window/SkinWnd.h"

using namespace Upp;

BEGIN_NAMESPACE_UPPEX

ParentCtrlStub::ParentCtrlStub() {
    hittest = HTTRANSPARENT;
}

ParentCtrlStub::~ParentCtrlStub() {
}

BOOL ParentCtrlStub::SetAttrParam(BOOL bSetPrev, Upp::String attrId, 
                                  Upp::String attrVal) {
    if(attrId == "layout") {
        //SetImage(theSkinMgr.ExtractImage(attrVal));
        String xmldata = theSkinMgr.ExtractLayoutXml(attrVal);
        BuildLayout(xmldata);
    }
    return TRUE;    

}

BOOL ParentCtrlStub::BuildLayout(String xmldata) {
    XmlNode xml = ParseXML(~xmldata);
    if(xml.IsEmpty())
        return FALSE;
    if(xml.GetCount() != 1)
        return FALSE;

    const XmlNode &rootNode = xml.At(0);
    if(!rootNode.IsTag("ParentCtrlStub"))
        return FALSE;

    for(int attr_i = 0 ; attr_i < rootNode.GetAttrCount() ; attr_i++) {
        String attrTag = rootNode.AttrId(attr_i);
        String attrVal = rootNode.Attr(attr_i);
    }
    
    for(int child_i = 0 ; child_i < rootNode.GetCount() ; child_i++) {
        if(!CreateChildCtrls(this , rootNode.Node(child_i))) {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL ParentCtrlStub::CreateChildCtrls(Upp::Ctrl *parent , const XmlNode &node) {
    const String& tag = node.GetTag();
    Upp::Ctrl* ctrl = CreateUppCtrlByTag(this, node);
    if(node.GetCount() > 0) {
        for(int child_i = 0 ; child_i < node.GetCount() ; child_i++) {
            if(!CreateChildCtrls(ctrl , node.Node(child_i))) {
                return FALSE;
            }
        }
    }
    return TRUE;
}


END_UPPEX_NAMESPACE
