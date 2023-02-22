#ifndef UPP_CTRLCREATOREX_H__
#define UPP_CTRLCREATOREX_H__

#include "UIHelper.h"

NAMESPACE_UPPEX
void InitializeUppCtrlCreators();
Upp::Ctrl* CreateUppCtrlByTag(Upp::Ctrl *parent , const Upp::XmlNode &node);
END_UPPEX_NAMESPACE
#endif