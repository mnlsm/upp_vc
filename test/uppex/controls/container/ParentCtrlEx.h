#ifndef UPPEX_ParentCtrlEx_H__
#define UPPEX_ParentCtrlEx_H__
#pragma once


#include "../../SkinMgr.h"

NAMESPACE_UPPEX
class ParentCtrlEx
        : public Upp::ParentCtrl {
public:
    ParentCtrlEx();
    virtual ~ParentCtrlEx();

public:
    virtual Upp::Image CursorImage(Upp::Point p, Upp::dword keyflags);
};

END_UPPEX_NAMESPACE
#endif