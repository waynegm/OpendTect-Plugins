#include "uimenu.h"
#include "uiodmain.h"
#include "odplugin.h"

#include "uimt_mai_sao.h"

mDefODPluginInfo(uiMT_MAI_SAO)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "Attribute skeleton (UI) for multi-trace, multi-attribute input and single-attribute output",
    "OpendTect",
    "Wayne Mogg",
    "1.0",
    "A starting point for writing an OpendTect attribute") );
    return &retpi;
}

mDefODInitPlugin(uiMT_MAI_SAO)
{
    uiMT_MAI_SAO::initClass();

    return 0;
}
