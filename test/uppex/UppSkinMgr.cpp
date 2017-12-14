#include "stdafx.h"
#include "UppSkinMgr.h"

using namespace Upp;
using namespace std::tr1;
using namespace stdext;
CUppSkinMgr::CUppSkinMgr() {

}

CUppSkinMgr::~CUppSkinMgr() {

}

/*
<skin name='xxxx'>
    <color id='color_1' value='0xFFFFFFFF'/>
    <image id='image_1' file='img\image_1.png'>
    <images file='img\image_1.png' margin='10,10,10,10'>
        <partimage id='image_1' sizerect='0,0,50,50'>
    </images>
</skin>
*/

BOOL CUppSkinMgr::LoadSkinFromXmlFile(const char *skinname , const char *xmlfile , bool filebom) {
    skinname_ = skinname;
    skinpath_ = Upp::GetFileFolder(xmlfile);
    String xmldata = filebom ? LoadFileBOM(xmlfile) : LoadFile(xmlfile);
    if(UppUIHelper::LoadResFromXml(~xmldata , skinpath_ , &resdata_)) {
        RefreshCtrlStyles();
        return TRUE;
    }
    return FALSE;
}

const char* CUppSkinMgr::GetName() {
    return ~skinname_;
}

Upp::Color CUppSkinMgr::ExtractColor(Upp::String id , Upp::byte alpha) {
    Color *clr = resdata_.colors_.FindPtr(id);
    if(clr != NULL) {
        if(alpha != 0xFF) {
            RGBA rgba = (RGBA) * clr;
            rgba.a = alpha;
            return Color(rgba);
        }
        return *clr;
    }
    return theResMgr.ExtractColor(id , alpha);
}

Upp::Image CUppSkinMgr::ExtractImage(Upp::String id) {
    Image *image = resdata_.images_.FindPtr(id);
    if(image != NULL)
        return *image;
    return theResMgr.ExtractImage(id);
}

Upp::Font CUppSkinMgr::ExtractFont(Upp::String id) {
    if(resdata_.fonts_.Find(id) >= 0) {
        return resdata_.fonts_.Get(id);
    }
    return GetStdFont();
}

const Button::Style& CUppSkinMgr::GetButtonStyle(const char* style_str) {
    static Button::Style style = Button::StyleNormal();
    CButtonStyleMap::iterator iter =  button_styles_.find(style_str);
    if(iter != button_styles_.end()) {
        return *iter->second.get();
    }
    return style;
}

const CUppSkinWnd::Style& CUppSkinMgr::GetUppSkinWndStyle(const char* style_str) {
    static CUppSkinWnd::Style style = CUppSkinWnd::StyleNormal();
    CWindowStyleMap::iterator iter = wnd_styles_.find(style_str);
    if(iter != wnd_styles_.end()) {
        return *iter->second.get();
    }
    return style;
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUppSkinMgr::RefreshCtrlStyles() {
    CButtonStyleMap::iterator iter = button_styles_.find("common_button");
    Button::Style *button_style = (iter != button_styles_.end()) ? iter->second.get() : new(std::nothrow) Button::Style();
    if(button_style != NULL) {
        button_style->Assign(Button::StyleNormal());
        button_style->look[0] = ExtractImage("common_button_normal");
        button_style->look[1] = ExtractImage("common_button_hover");
        button_style->look[2] = ExtractImage("common_button_down");
        button_style->look[3] = ExtractImage("common_button_disable");
        if(iter == button_styles_.end())
            button_styles_["common_button"] = shared_ptr< Button::Style >(button_style);
    }

    iter = button_styles_.find("sys_sizebtn");
    Button::Style *sizebox_style = (iter != button_styles_.end()) ? iter->second.get() : new(std::nothrow) Button::Style();
    if(sizebox_style != NULL) {
        sizebox_style->Assign(Button::StyleNormal());
        sizebox_style->look[0] = ExtractImage("sys_sizebox");
        sizebox_style->look[1] = ExtractImage("sys_sizebox");
        sizebox_style->look[2] = ExtractImage("sys_sizebox");
        sizebox_style->look[3] = ExtractImage("sys_sizebox");
        if(iter == button_styles_.end())
            button_styles_["sys_sizebtn"] = shared_ptr< Button::Style >(sizebox_style);
    }

    iter = button_styles_.find("sys_closebtn");
    Button::Style *sys_close_style = (iter != button_styles_.end()) ? iter->second.get() : new(std::nothrow) Button::Style();
    if(sys_close_style != NULL) {
        sys_close_style->Assign(Button::StyleNormal());
        sys_close_style->look[0] = ExtractImage("sys_close_normal");
        sys_close_style->look[1] = ExtractImage("sys_close_hover");
        sys_close_style->look[2] = ExtractImage("sys_close_down");
        sys_close_style->look[3] = ExtractImage("sys_close_disable");
        if(iter == button_styles_.end())
            button_styles_["sys_closebtn"] = shared_ptr< Button::Style >(sys_close_style);
    }

    iter = button_styles_.find("sys_minbtn");
    Button::Style *sys_min_style = (iter != button_styles_.end()) ? iter->second.get() : new(std::nothrow) Button::Style();
    if(sys_min_style != NULL) {
        sys_min_style->Assign(Button::StyleNormal());
        sys_min_style->look[0] = ExtractImage("sys_min_normal");
        sys_min_style->look[1] = ExtractImage("sys_min_hover");
        sys_min_style->look[2] = ExtractImage("sys_min_down");
        sys_min_style->look[3] = ExtractImage("sys_min_disable");
        if(iter == button_styles_.end())
            button_styles_["sys_minbtn"] = shared_ptr< Button::Style >(sys_min_style);
    }

    iter = button_styles_.find("sys_maxbtn");
    Button::Style *sys_max_style = (iter != button_styles_.end()) ? iter->second.get() : new(std::nothrow) Button::Style();
    if(sys_max_style != NULL) {
        sys_max_style->Assign(Button::StyleNormal());
        sys_max_style->look[0] = ExtractImage("sys_max_normal");
        sys_max_style->look[1] = ExtractImage("sys_max_hover");
        sys_max_style->look[2] = ExtractImage("sys_max_down");
        sys_max_style->look[3] = ExtractImage("sys_max_disable");
        if(iter == button_styles_.end())
            button_styles_["sys_maxbtn"] = shared_ptr< Button::Style >(sys_max_style);
    }

    iter = button_styles_.find("sys_restorebtn");
    Button::Style *sys_restore_style = (iter != button_styles_.end()) ? iter->second.get() : new(std::nothrow) Button::Style();
    if(sys_restore_style != NULL) {
        sys_restore_style->Assign(Button::StyleNormal());
        sys_restore_style->look[0] = ExtractImage("sys_restore_normal");
        sys_restore_style->look[1] = ExtractImage("sys_restore_hover");
        sys_restore_style->look[2] = ExtractImage("sys_restore_down");
        sys_restore_style->look[3] = ExtractImage("sys_restore_disable");
        if(iter == button_styles_.end())
            button_styles_["sys_restorebtn"] = shared_ptr< Button::Style >(sys_restore_style);
    }


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CWindowStyleMap::iterator wnditer = wnd_styles_.find("wnd_background_grid");
    CUppSkinWnd::Style *uppskinwnd_grid = (wnditer != wnd_styles_.end()) ? wnditer->second.get() : new(std::nothrow) CUppSkinWnd::Style();
    if(uppskinwnd_grid) {
        uppskinwnd_grid->image_[0] = ExtractImage("wnd_topleft");
        uppskinwnd_grid->image_[1] = ExtractImage("wnd_topmiddle");
        uppskinwnd_grid->image_[2] = ExtractImage("wnd_topright");
        uppskinwnd_grid->image_[3] = ExtractImage("wnd_centerleft");
        uppskinwnd_grid->image_[4] = ExtractImage("wnd_centermiddle");
        uppskinwnd_grid->image_[5] = ExtractImage("wnd_centerright");
        uppskinwnd_grid->image_[6] = ExtractImage("wnd_bottomleft");
        uppskinwnd_grid->image_[7] = ExtractImage("wnd_bottommiddle");
        uppskinwnd_grid->image_[8] = ExtractImage("wnd_bottomright");
        uppskinwnd_grid->bkgnd_type_ = CUppSkinWnd::Style::GRID;
        if(wnditer == wnd_styles_.end())
            wnd_styles_["wnd_background_grid"] = shared_ptr< CUppSkinWnd::Style >(uppskinwnd_grid);
    }

    wnditer = wnd_styles_.find("wnd_background_color_grid");
    CUppSkinWnd::Style *uppskinwnd_color_grid = (wnditer != wnd_styles_.end()) ? wnditer->second.get() : new(std::nothrow) CUppSkinWnd::Style();
    if(uppskinwnd_grid) {
        uppskinwnd_color_grid->image_[0] = ExtractImage("wnd_color_topleft");
        uppskinwnd_color_grid->image_[1] = ExtractImage("wnd_color_topmiddle");
        uppskinwnd_color_grid->image_[2] = ExtractImage("wnd_color_topright");
        uppskinwnd_color_grid->image_[3] = ExtractImage("wnd_color_centerleft");
        uppskinwnd_color_grid->image_[4] = ExtractImage("wnd_color_centermiddle");
        uppskinwnd_color_grid->image_[5] = ExtractImage("wnd_color_centerright");
        uppskinwnd_color_grid->image_[6] = ExtractImage("wnd_color_bottomleft");
        uppskinwnd_color_grid->image_[7] = ExtractImage("wnd_color_bottommiddle");
        uppskinwnd_color_grid->image_[8] = ExtractImage("wnd_color_bottomright");
        uppskinwnd_color_grid->bkgnd_type_ = CUppSkinWnd::Style::GRID;
        if(wnditer == wnd_styles_.end())
            wnd_styles_["wnd_background_color_grid"] = shared_ptr< CUppSkinWnd::Style >(uppskinwnd_color_grid);
    }


}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUppSkinMgr& CUppSkinMgr::GetInstance() {
    static CUppSkinMgr *p;
    ONCELOCK {
        static CUppSkinMgr o;
        p = &o;
    }
    return *p;
}
