#include "stdafx.h"
#include "CtrlCreator.h"
#include "UIHelper.h"
#include <assert.h>

#include "controls/container/ParentCtrlEx.h"
#include "controls/static/ImageCtrlEx.h"


namespace {
class ICtrlCreator 
    :public Upp::PteBase {
protected:
    ICtrlCreator(){}
    virtual ~ICtrlCreator() {}

public:
    virtual Upp::Ctrl* Create() = 0;
};
typedef Upp::Ptr<ICtrlCreator> CtrlCreatorPtr;

template<class T> class CtrlCreator
        :public ICtrlCreator {
public:
    CtrlCreator(){}
    virtual ~CtrlCreator() {}

public:
    virtual Upp::Ctrl* Create() {
        return new (std::nothrow) T();
    }
};
}



///////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Upp;

static Upp::VectorMap<Upp::String , CtrlCreatorPtr>& GetCache() {
    static Upp::VectorMap<Upp::String , CtrlCreatorPtr> *p;
    ONCELOCK {
        static Upp::VectorMap<Upp::String , CtrlCreatorPtr> o;
        p = &o;
    }
    return *p;
}

NAMESPACE_UPPEX
void InitializeUppCtrlCreators() {
    GetCache().Add("ParentCtrlEx", new (std::nothrow) CtrlCreator<ParentCtrlEx>());
    GetCache().Add("Button", new (std::nothrow) CtrlCreator<Upp::Button>());
    GetCache().Add("EditField", new (std::nothrow) CtrlCreator<Upp::EditField>());
    GetCache().Add("WithDropChoice", new (std::nothrow) CtrlCreator<Upp::WithDropChoice<EditString>>());
    GetCache().Add("ImageCtrlEx", new (std::nothrow) CtrlCreator<ImageCtrlEx>());
}


Upp::Ctrl* CreateUppCtrlByTag(Ctrl *parent, const XmlNode &node) {
    const String& tag = node.GetTag();
    CtrlCreatorPtr* creator = GetCache().FindPtr(tag);
    if(creator == NULL) {
        assert(false);
        return NULL;
    }
    Upp::Ctrl* ctrl = (*creator)->Create();
    if(ctrl == NULL) {
        assert(false);
        return NULL;
    }
    Size sz(0 , 0);
    Point pt(0 , 0);
    ctrl->WantFocus(false);
    for(int i = 0 ; i < node.GetAttrCount() ; i++) {
        const String &attrId = node.AttrId(i);
        const String &attrVal = node.Attr(i);
        BOOL ret = UIHelper::SetCtrlParam(ctrl, attrId, attrVal);
        ret = ctrl->SetAttrParam(ret, attrId, attrVal);
        if(!ret) {
            ASSERT(FALSE);
            return FALSE;
        }    
    }
    parent->AddChild(ctrl);
    parent->GetTopCtrl()->OnSubCtrlCreate(ctrl, node);    
    return ctrl;
}

END_UPPEX_NAMESPACE