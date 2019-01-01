#include "uimenu.h"
#include "uiodmain.h"
#include "odplugin.h"

#include "uimt_mai_mao.h"

mDefODPluginInfo(uiMT_MAI_MAO)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "Attribute skeleton (UI) for multi-trace, multi-attribute input and multi-attribute output",
    "OpendTect",
    "Wayne Mogg",
    "1.0",
    "A starting point for writing an OpendTect attribute") );
    return &retpi;
}

mDefODInitPlugin(uiMT_MAI_MAO)
{
    uiMT_MAI_MAO::initClass();

    return 0;
}
