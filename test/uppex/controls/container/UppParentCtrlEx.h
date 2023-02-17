#ifndef UPPEX_UPPPARENTCTRLEX_H__
#define UPPEX_UPPPARENTCTRLEX_H__
#pragma once


#include "../../UppSkinMgr.h"

NAMESPACE_UPPEX
class UppParentCtrlEx
        : public Upp::ParentCtrl {
public:
    UppParentCtrlEx() {
    };
    ~UppParentCtrlEx() {
    };

public:
    virtual Upp::Image CursorImage(Upp::Point p, Upp::dword keyflags);
};

END_UPPEX_NAMESPACE
#endif