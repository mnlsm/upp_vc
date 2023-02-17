#ifndef UPPEX_UPPSTATICEX_H__
#define UPPEX_UPPSTATICEX_H__

#include "../../UppSkinMgr.h"

NAMESPACE_UPPEX
class UppStaticEx
        : public Upp::ParentCtrl {
public:
    UppStaticEx();
    virtual ~UppStaticEx();

public:
    //virtual Upp::Image CursorImage(Upp::Point p, Upp::dword keyflags);
};

END_UPPEX_NAMESPACE
#endif