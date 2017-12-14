#ifndef UPPRESMGR_H__
#define UPPRESMGR_H__

#include <core/core.h>
#include <draw/Draw.h>
#include <ctrlcore/SystemDraw.h>
#include <ctrllib/CtrlLib.h>
#include "UppUIHelper.h"




class CUppResMgr
        : Upp::NoCopy {
private:
    CUppResMgr();
    ~CUppResMgr();

public:
    BOOL LoadResXmlFile(const char *resfile , bool filebom);

public:
    Upp::Color ExtractColor(Upp::String id , Upp::byte alpha = 0xFF);
    Upp::Image ExtractImage(Upp::String id);
    Upp::Font  ExtractFont(Upp::String id);


public:
    static CUppResMgr& GetInstance();



private:
    UppResData resdata_;



private:
    Upp::String respath_;
};

#define theResMgr CUppResMgr::GetInstance()

#endif