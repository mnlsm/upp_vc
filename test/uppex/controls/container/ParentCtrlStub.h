#ifndef UPPEX_ParentCtrlStub_H__
#define UPPEX_ParentCtrlStub_H__
#pragma once


#include "../../SkinMgr.h"

BEGIN_NAMESPACE_UPPEX
class ParentCtrlStub
    : public Upp::ParentCtrl {

public:
    ParentCtrlStub();
    virtual ~ParentCtrlStub();

public:
    virtual BOOL SetAttrParam(BOOL bSetPrev, Upp::String attrId, Upp::String attrVal);

protected:
    virtual BOOL BuildLayout(Upp::String xmldata);
    virtual BOOL CreateChildCtrls(Upp::Ctrl *parent , const Upp::XmlNode &node);


};



END_UPPEX_NAMESPACE

#endif
