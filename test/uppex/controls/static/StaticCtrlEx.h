#ifndef UPPEX_StaticCtrlEx_H__
#define UPPEX_StaticCtrlEx_H__

#include "../../SkinMgr.h"

NAMESPACE_UPPEX
class StaticCtrlEx
        : public Upp::Label {
public:
    StaticCtrlEx();
    virtual ~StaticCtrlEx();

public:
    virtual void Paint(Upp::Draw& w);

public:
    virtual BOOL SetAttrParam(BOOL bSetPrev, Upp::String attrId, Upp::String attrVal);
};

END_UPPEX_NAMESPACE
#endif