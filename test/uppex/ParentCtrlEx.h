#ifndef UPP_PARENTCTRLEX_H__
#define UPP_PARENTCTRLEX_H__

#include <CtrlCore/CtrlCore.h>
#include <CtrlLib/CtrlLib.h>
#include <draw/draw.h>

#include "UppUIHelper.h"

class ParentCtrlEx
        : public Upp::ParentCtrl {
public:
    ParentCtrlEx() {};
    ~ParentCtrlEx() {};

public:
    virtual Upp::Image CursorImage(Upp::Point p, Upp::dword keyflags);
};


#endif