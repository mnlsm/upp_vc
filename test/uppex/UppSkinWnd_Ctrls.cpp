#include "stdafx.h"
#include "UppSkinWnd.h"
#include "UppSkinMgr.h"
#include "ParentCtrlEx.h"


using namespace Upp;
#define LLOG(x)

static Ctrl* GetSubCtrlByLayoutId(Ctrl* ctrl , const char *id) {
    for(Ctrl *subctrl = ctrl->GetFirstChild(); subctrl != NULL ; subctrl = subctrl->GetNext()) {
        if(subctrl->GetLayoutId() == id)
            return subctrl;
        Ctrl* ret = GetSubCtrlByLayoutId(subctrl , id);
        if(ret != NULL)
            return ret;
    }
    return NULL;
}

Upp::Ctrl* CUppSkinWnd::GetCtrlByLayoutId(const char *id) {
    if(GetLayoutId() == id) {
        return this;
    }
    std::map<std::string, Upp::Ptr<Upp::Ctrl>>::iterator iter = vCacheCtrls_.find(id);
    if(iter != vCacheCtrls_.end()) {
        return ~iter->second;    
    }
    return GetSubCtrlByLayoutId(this , id);
}

template<class T> T* GetOrCreateCtrl(const XmlNode &node , CUppSkinWnd *wnd) {
    const String &layid = node.Attr("layid");
    const String &layidc = node.Attr("layidc");
#ifdef _DEBUG
    if(!layid.IsEmpty()) {
        Ctrl *ctrl = wnd->GetCtrlByLayoutId(layid);
        if(ctrl != NULL) {
            ASSERT(FALSE);
            return dynamic_cast<T*>(ctrl);
        }
    }
    if(!layidc.IsEmpty()) {
        Ctrl *ctrl = wnd->GetCtrlByLayoutId(layidc);
        if(ctrl != NULL) {
            ASSERT(FALSE);
            return dynamic_cast<T*>(ctrl);
        }
    }
#endif
    if(!layid.IsEmpty() && !layidc.IsEmpty()) {
        ASSERT(FALSE);
    }
    return new(std::nothrow) T();
}

BOOL CUppSkinWnd::SetCtrlParams(Upp::Ctrl* ctrl , const XmlNode &node) {
    Size sz(0 , 0);
    Point pt(0 , 0);
    ctrl->WantFocus(false);
    for(int i = 0 ; i < node.GetAttrCount() ; i++) {
        const String &attrId = node.AttrId(i);
        const String &attrVal = node.Attr(i);
        if(attrId == "layid") {
            ctrl->LayoutId(attrVal);
        } else if(attrId == "layidc") {
            ctrl->LayoutId(attrVal);
            vCacheCtrls_[to_string(attrVal)] = Upp::Ptr<Upp::Ctrl>(ctrl);
        } else if(attrId == "tip") {
            ctrl->Tip(attrVal);
        }
        else if(attrId == "tabstop") {
            ctrl->WantFocus(UppUIHelper::ParseBool(attrVal));
        }else if(attrId == "HSizePos") {
            sz = UppUIHelper::ParseSize(attrVal);
            ctrl->HSizePos(sz.cx , sz.cy);
        }else if(attrId == "HSizePosZ") {
            sz = UppUIHelper::ParseSize(attrVal);
            ctrl->HSizePosZ(sz.cx , sz.cy);
        }else if(attrId == "VSizePos") {
            sz = UppUIHelper::ParseSize(attrVal);
            ctrl->VSizePos(sz.cx , sz.cy);
        }else if(attrId == "VSizePosZ") {
            sz = UppUIHelper::ParseSize(attrVal);
            ctrl->VSizePosZ(sz.cx , sz.cy);
        }else if(attrId == "HCenterPos") {
            pt = UppUIHelper::ParsePoint(attrVal);
            ctrl->HCenterPos(pt.x , pt.y);
        }else if(attrId == "HCenterPosZ") {
            pt = UppUIHelper::ParsePoint(attrVal);
            ctrl->HCenterPosZ(pt.x , pt.y);
        }else if(attrId == "VCenterPos") {
            pt = UppUIHelper::ParsePoint(attrVal);
            ctrl->VCenterPos(pt.x , pt.y);
        }else if(attrId == "VCenterPosZ") {
            pt = UppUIHelper::ParsePoint(attrVal);
            ctrl->VCenterPosZ(pt.x , pt.y);
        }else if(attrId == "LeftPos") {
            pt = UppUIHelper::ParsePoint(attrVal);
            ctrl->LeftPos(pt.x , pt.y);
        }else if(attrId == "LeftPosZ") {
            pt = UppUIHelper::ParsePoint(attrVal);
            ctrl->LeftPosZ(pt.x , pt.y);
        }else if(attrId == "RightPos") {
            pt = UppUIHelper::ParsePoint(attrVal);
            ctrl->RightPos(pt.x , pt.y);
        }else if(attrId == "RightPosZ") {
            pt = UppUIHelper::ParsePoint(attrVal);
            ctrl->RightPosZ(pt.x , pt.y);
        }else if(attrId == "TopPos") {
            pt = UppUIHelper::ParsePoint(attrVal);
            ctrl->TopPos(pt.x , pt.y);
        }else if(attrId == "TopPosZ") {
            pt = UppUIHelper::ParsePoint(attrVal);
            ctrl->TopPosZ(pt.x , pt.y);
        }else if(attrId == "BottomPos") {
            pt = UppUIHelper::ParsePoint(attrVal);
            ctrl->BottomPos(pt.x , pt.y);
        }else if(attrId == "BottomPosZ") {
            pt = UppUIHelper::ParsePoint(attrVal);
            ctrl->BottomPosZ(pt.x , pt.y);
        }
    }
    if(ctrl->GetLayoutId().IsEmpty()) {
        ATLASSERT(FALSE);
        return FALSE;
    }
    return TRUE;
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
    if(tag == "ParentCtrl") {
        Ptr<ParentCtrl> parentCtrl = GetOrCreateCtrl<ParentCtrl>(node , this);
        if(~parentCtrl == NULL) return FALSE;
        if(!SetCtrlParams(parentCtrl , node)) return FALSE;
        for(int child_i = 0 ; child_i < node.GetCount() ; child_i++) {
            if(!CreateChildCtrls(~parentCtrl , node.Node(child_i)))
                return FALSE;
        }
        parent->AddChild(~parentCtrl);
    }
    if(tag == "ParentCtrlEx") {
        Ptr<ParentCtrlEx> parentCtrl = GetOrCreateCtrl<ParentCtrlEx>(node , this);
        if(~parentCtrl == NULL) return FALSE;
        if(!SetCtrlParams(parentCtrl , node)) return FALSE;
        String layid = parentCtrl->GetLayoutId();
        if(layid == "sys_titlebar") titlebar_ = parentCtrl;
        for(int child_i = 0 ; child_i < node.GetCount() ; child_i++) {
            if(!CreateChildCtrls(~parentCtrl , node.Node(child_i)))
                return FALSE;
        }
        parent->AddChild(~parentCtrl);
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
    }

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