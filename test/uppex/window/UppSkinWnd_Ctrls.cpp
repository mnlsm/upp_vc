#include "stdafx.h"
#include "UppSkinWnd.h"
#include "../UppSkinMgr.h"

//#include "ParentCtrlEx.h"


using namespace Upp;
#define LLOG(x)

static Ctrl* GetSubCtrlByLayoutId(Ctrl* ctrl , const char *id) {
    for(Ctrl *subctrl = ctrl->GetFirstChild(); subctrl != NULL ; subctrl = subctrl->GetNext()) {
        String idd = subctrl->GetLayoutId();
        if(idd == id)
            return subctrl;
        Ctrl* ret = GetSubCtrlByLayoutId(subctrl , id);
        if(ret != NULL)
            return ret;
    }
    return NULL;
}

NAMESPACE_UPPEX
Upp::Ctrl* CUppSkinWnd::GetCtrlByLayoutId(const char *id) {
    if(GetLayoutId() == id) {
        return this;
    }
    Upp::Ptr<Upp::Ctrl>* ptr = vCacheCtrls_.FindPtr(id);
    if(ptr != NULL) {
        return ~(*ptr);    
    }
    return GetSubCtrlByLayoutId(this , id);
}

void CUppSkinWnd::OnSubCtrlCreate(Ctrl* ctrl, const Upp::XmlNode &node) {
    const String& tag = node.GetTag();
    String layidc = node.Attr("layidc");
    if(!layidc.IsEmpty()) {
        ctrl->LayoutId(layidc);
        vCacheCtrls_.Add(layidc, Upp::Ptr<Upp::Ctrl>(ctrl));        
    }
    subctrls_.push_back(std::tr1::shared_ptr<Upp::Ctrl>(ctrl));
    if(tag == "Button") {
        Upp::Button* button = dynamic_cast<Upp::Button*>(ctrl);
        String attrVal = node.Attr("label");
        if(!attrVal.IsEmpty()) button->SetLabel(attrVal);
        String layid = ctrl->GetLayoutId();
        if(layid == "sys_closebtn") {
            String skin = node.Attr("skin");
            UpdateSysButtonStyle(button, skin);
            button->NoWantFocus();
            closeboxbtn_ = button;
            NoCloseBox(false);
        }else if(layid == "sys_minbtn") {
            String skin = node.Attr("skin");
            UpdateSysButtonStyle(button, skin);
            button->NoWantFocus();
            minboxbtn_ = button;
            minboxbtn_->NoWantFocus();
            MinimizeBox(true);
        }else if(layid == "sys_maxbtn") {
            String skin = node.Attr("skin");
            UpdateSysButtonStyle(button, skin);
            button->NoWantFocus();
            maxboxbtn_ = button;
            MaximizeBox(true);
        }else if(layid == "sys_restorebtn") {
            String skin = node.Attr("skin");
            UpdateSysButtonStyle(button, skin);
            button->NoWantFocus();
            restoreboxbtn_ = button;
        }else if(layid == "sys_sizebtn") {
            String skin = node.Attr("skin");
            UpdateSysButtonStyle(button, skin);
            button->NoWantFocus();
            sizeboxbtn_ = button;
            sizeboxbtn_->Show(true);
            sizeboxbtn_->Enable(false);
        }
    }
}



BOOL CUppSkinWnd::CreateChildCtrls(Upp::Ctrl *parent , const XmlNode &node) {
    const String& tag = node.GetTag();
    if(tag == "wndclippings") {
        if(!skinalpha_) {
            vClippings_.reserve(1280);
            ResetClippings(node);
        }
        return TRUE;
    }
    Upp::Ctrl* ctrl = CreateUppCtrlByTag(this, node);
    if(node.GetCount() > 0) {
        for(int child_i = 0 ; child_i < node.GetCount() ; child_i++) {
            if(!CreateChildCtrls(ctrl , node.Node(child_i))) {
                return FALSE;
            }
        }
    }

/*
    if(tag == "ParentCtrl") {
        Ptr<ParentCtrl> parentCtrl = GetOrCreateCtrl<ParentCtrl>(node , this);
        if(~parentCtrl == NULL) return FALSE;
        if(!SetCtrlParams(parentCtrl , node)) return FALSE;
        for(int child_i = 0 ; child_i < node.GetCount() ; child_i++) {
            if(!CreateChildCtrls(~parentCtrl , node.Node(child_i)))
                return FALSE;
        }
        parent->AddChild(~parentCtrl);
        subctrls_.push_back(std::tr1::shared_ptr<Upp::Ctrl>(~parentCtrl));
    }
    if(tag == "ParentCtrlEx") {
        //Ptr<ParentCtrlEx> parentCtrl = GetOrCreateCtrl<ParentCtrlEx>(node , this);
        ParentCtrlEx* parentCtrl = GetOrCreateCtrl<ParentCtrlEx>(node , this);
        //parentCtrl.      
        if(parentCtrl == NULL) return FALSE;
        if(!SetCtrlParams(parentCtrl , node)) return FALSE;
        String layid = parentCtrl->GetLayoutId();
        if(layid == "sys_titlebar") titlebar_ = parentCtrl;
        for(int child_i = 0 ; child_i < node.GetCount() ; child_i++) {
            if(!CreateChildCtrls(parentCtrl , node.Node(child_i)))
                return FALSE;
        }
        parent->AddChild(parentCtrl);
        subctrls_.push_back(std::tr1::shared_ptr<Upp::Ctrl>(parentCtrl));
    }else if(tag == "Button") {
        Ptr<Button> button = GetOrCreateCtrl<Button>(node , this);
        if(~button == NULL) return FALSE;
        if(!SetCtrlParams(button , node)) return FALSE;
        String layid = button->GetLayoutId();
        if(layid == "sys_closebtn") {
            button->NoWantFocus();
            closeboxbtn_ = button;
            NoCloseBox(false);
        }else if(layid == "sys_minbtn") {
            button->NoWantFocus();
            minboxbtn_ = button;
            minboxbtn_->NoWantFocus();
            MinimizeBox(true);
        }else if(layid == "sys_maxbtn") {
            button->NoWantFocus();
            maxboxbtn_ = button;
            MaximizeBox(true);
        }else if(layid == "sys_restorebtn") {
            button->NoWantFocus();
            restoreboxbtn_ = button;
        }else if(layid == "sys_sizebtn") {
            button->NoWantFocus();
            sizeboxbtn_ = button;
            sizeboxbtn_->Show(true);
            sizeboxbtn_->Enable(false);
        }
        String skin = node.Attr("skin");
        if(!skin.IsEmpty()) button->SetStyle(theSkinMgr.GetButtonStyle(skin));
        else button->SetStyle(theSkinMgr.GetButtonStyle("normal"));
        String attrVal = node.Attr("label");
        if(!attrVal.IsEmpty()) button->SetLabel(attrVal);
        parent->AddChild(~button);
    }else if(tag == "EditField") {
        Ptr<EditField> edit = GetOrCreateCtrl<EditField>(node , this);
        if(~edit == NULL) return FALSE;
        if(!SetCtrlParams(edit , node)) return FALSE;
//        edit->SetFrame( NullFrame() );
        edit->NoBackground(true);
        parent->AddChild(~edit);
    }else if(tag == "WithDropChoice") {
        Ptr< WithDropChoice< EditString > > withdropchoice = GetOrCreateCtrl< WithDropChoice< EditString > >(node , this);
        if(~withdropchoice == NULL) return FALSE;
        if(!SetCtrlParams(withdropchoice , node)) return FALSE;
        withdropchoice->AddList("ssss");
        withdropchoice->AddList("aaaa");
        withdropchoice->SetLineCy(20);
        withdropchoice->SetFocus();
        parent->AddChild(~withdropchoice);
    }*/

    return TRUE;
}
// transcolor='255,0,255'
void CUppSkinWnd::ResetClippings(const Upp::XmlNode &node) {
    calc_skin_rgn_ = false;
    for(int i = 0 ; i < node.GetCount() ; i++) {
        const Upp::XmlNode &item = node[i];
        if(item.GetTag() == "item") {
            ClipRegionData clip;
            Point pt(0 , 0);
            for(int ai = 0 ; ai < item.GetAttrCount() ; ai++) {
                const String &attrId = item.AttrId(ai);
                const String &attrVal = item.Attr(ai);
                if(attrId == "LeftPos") {
                    pt = UppUIHelper::ParsePoint(attrVal);
                    clip.pos.x = Ctrl::PosLeft(pt.x , pt .y);
                }else if(attrId == "RightPos") {
                    pt = UppUIHelper::ParsePoint(attrVal);
                    clip.pos.x = Ctrl::PosRight(pt.x , pt .y);
                }else if(attrId == "TopPos") {
                    pt = UppUIHelper::ParsePoint(attrVal);
                    clip.pos.y = Ctrl::PosTop(pt.x , pt .y);
                }else if(attrId == "BottomPos") {
                    pt = UppUIHelper::ParsePoint(attrVal);
                    clip.pos.y = Ctrl::PosBottom(pt.x , pt .y);
                }
            }
            vClippings_.push_back(clip);
        }
    }
}
END_UPPEX_NAMESPACE