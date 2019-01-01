#include "uimenu.h"
#include "uiodmain.h"
#include "odplugin.h"

#include "uist_mai_mao.h"

mDefODPluginInfo(uiST_MAI_MAO)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "Attribute skeleton (UI) for single-trace, multi-attribute input and multi-attribute output",
    "OpendTect",
    "Wayne Mogg",
    "1.0",
    "A starting point for writing an OpendTect attribute") );
    return &retpi;
}

mDefODInitPlugin(uiST_MAI_MAO)
{
    uiST_MAI_MAO::initClass();

    return 0;
}
