#ifndef UPP_UppParentCtrlEx_H__
#define UPP_UppParentCtrlEx_H__


#include "../../UppUIHelper.h"

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