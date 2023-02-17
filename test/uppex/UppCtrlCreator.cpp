#include "stdafx.h"
#include "UppCtrlCreator.h"
#include "UppUIHelper.h"
#include <assert.h>

#include "controls/container/UppParentCtrlEx.h"
#include "controls/static/UppImageCtrlEx.h"


namespace {
class IUppCtrlCreator 
    :public Upp::PteBase {
protected:
    IUppCtrlCreator(){}
    virtual ~IUppCtrlCreator() {}

public:
    virtual Upp::Ctrl* Create() = 0;
};
typedef Upp::Ptr<IUppCtrlCreator> UppCtrlCreatorPtr;

template<class T> class UppCtrlCreator
        :public IUppCtrlCreator {
public:
    UppCtrlCreator(){}
    virtual ~UppCtrlCreator() {}

public:
    virtual Upp::Ctrl* Create() {
        return new (std::nothrow) T();
    }
};
}



///////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Upp;

static Upp::VectorMap<Upp::String , UppCtrlCreatorPtr>& GetCache() {
    static Upp::VectorMap<Upp::String , UppCtrlCreatorPtr> *p;
    ONCELOCK {
        static Upp::VectorMap<Upp::String , UppCtrlCreatorPtr> o;
        p = &o;
    }
    return *p;
}

NAMESPACE_UPPEX
void InitializeUppCtrlCreators() {
    GetCache().Add("ParentCtrlEx", new (std::nothrow) UppCtrlCreator<UppParentCtrlEx>());
    GetCache().Add("Button", new (std::nothrow) UppCtrlCreator<Upp::Button>());
    GetCache().Add("EditField", new (std::nothrow) UppCtrlCreator<Upp::EditField>());
    GetCache().Add("WithDropChoice", new (std::nothrow) UppCtrlCreator<Upp::WithDropChoice<EditString>>());
    GetCache().Add("ImageCtrlEx", new (std::nothrow) UppCtrlCreator<UppImageCtrlEx>());
}


Upp::Ctrl* CreateUppCtrlByTag(Ctrl *parent, const XmlNode &node) {
    const String& tag = node.GetTag();
    UppCtrlCreatorPtr* creator = GetCache().FindPtr(tag);
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
        BOOL ret = UppUIHelper::SetCtrlParam(ctrl, attrId, attrVal);
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