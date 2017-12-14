#ifndef UPPSKINMGR_H__
#define UPPSKINMGR_H__
#include <core/core.h>
#include <draw/Draw.h>
#include <ctrlcore/SystemDraw.h>
#include <ctrllib/CtrlLib.h>
#include "UppUIHelper.h"
#include "UppResMgr.h"

#include "UppSkinWnd.h"



class CUppSkinWnd;

class CUppSkinMgr
        : Upp::NoCopy {
private:
    CUppSkinMgr();
    ~CUppSkinMgr();

public:
    static CUppSkinMgr& GetInstance();


public:
    BOOL LoadSkinFromXmlFile(const char *skinname , const char *xmlfile , bool filebom);


public:
    const char* GetName();
    Upp::Color ExtractColor(Upp::String id , Upp::byte alpha = 0xFF);
    Upp::Image ExtractImage(Upp::String id);
    Upp::Font  ExtractFont(Upp::String id);


public:
    const Upp::Button::Style& GetButtonStyle(const char* style_str);
    const CUppSkinWnd::Style& GetUppSkinWndStyle(const char* style_str);


private:
    void RefreshCtrlStyles();

private:
    UppResData resdata_;

    typedef stdext::hash_map< std::string , std::tr1::shared_ptr<Upp::Button::Style> > CButtonStyleMap;
    CButtonStyleMap button_styles_;
    typedef stdext::hash_map< std::string , std::tr1::shared_ptr< CUppSkinWnd::Style > > CWindowStyleMap;
    CWindowStyleMap wnd_styles_;

    Upp::String skinname_;
    Upp::String skinpath_;

};



#define theSkinMgr CUppSkinMgr::GetInstance()

#endif