#ifndef UPPEX_UPPIMAGECTRLEX_H__
#define UPPEX_UPPIMAGECTRLEX_H__

#include "../../UppSkinMgr.h"

NAMESPACE_UPPEX

class UppImageCtrlEx
        : public Upp::ImageCtrl {
public:
    UppImageCtrlEx();
    virtual ~UppImageCtrlEx();

public:
    virtual void Paint(Upp::Draw& w);

public:
    //virtual Upp::Image CursorImage(Upp::Point p, Upp::dword keyflags);
    virtual BOOL SetAttrParam(BOOL bSetPrev, Upp::String attrId, Upp::String attrVal);

};

END_UPPEX_NAMESPACE
#endif