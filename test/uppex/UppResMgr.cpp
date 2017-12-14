#include "stdafx.h"
#include "UppResMgr.h"

using namespace Upp;
using namespace std::tr1;
using namespace stdext;

CUppResMgr::CUppResMgr() {

}

CUppResMgr::~CUppResMgr() {

}

BOOL CUppResMgr::LoadResXmlFile(const char *resfile , bool filebom) {
    respath_ = Upp::GetFileFolder(resfile);
    String xmldata = filebom ? LoadFileBOM(resfile) : LoadFile(resfile);
    return UppUIHelper::LoadResFromXml(~xmldata , respath_ , &resdata_);
}

Upp::Color CUppResMgr::ExtractColor(Upp::String id , Upp::byte alpha) {
    Color clr(0 , 0 , 0);
    clr = resdata_.colors_.Get(id , clr);
    if(alpha != 0xFF) {
        RGBA rgba = (RGBA)clr;
        rgba.a = alpha;
        return Color(rgba);
    }
    return clr;
}

Upp::Image CUppResMgr::ExtractImage(Upp::String id) {
    Image image = resdata_.images_.Get(id);
    if(image.IsEmpty()) {
        ImageBuffer buf(1 , 1);
        buf[0]->r = buf[0]->g = buf[0]->b = 0x00;
        buf[0]->a = 0xFF;
        image = buf;
    }
    return image;
}

Upp::Font CUppResMgr::ExtractFont(Upp::String id) {
    if(resdata_.fonts_.Find(id) >= 0) {
        return resdata_.fonts_.Get(id);
    }
    return GetStdFont();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUppResMgr& CUppResMgr::GetInstance() {
    static CUppResMgr *p;
    ONCELOCK {
        static CUppResMgr o;
        p = &o;
    }
    return *p;
}