#ifndef UPPEX_ImageCtrlEx_H__
#define UPPEX_ImageCtrlEx_H__

#include "../../SkinMgr.h"

NAMESPACE_UPPEX

class ImageCtrlEx
        : public Upp::ImageCtrl {
public:
    ImageCtrlEx();
    virtual ~ImageCtrlEx();

public:
    virtual void Paint(Upp::Draw& w);

public:
    //virtual Upp::Image CursorImage(Upp::Point p, Upp::dword keyflags);
    virtual BOOL SetAttrParam(BOOL bSetPrev, Upp::String attrId, Upp::String attrVal);

};

END_UPPEX_NAMESPACE
#endif