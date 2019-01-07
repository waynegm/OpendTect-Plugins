#include "mt_mai_mao.h"
#include "odplugin.h"
#include "mt_mai_maomod.h"


mDefODPluginEarlyLoad(MT_MAI_MAO)
mDefODPluginInfo(MT_MAI_MAO)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "Attribute skeleton for multi-trace, multi-attribute input and multi-attribute output",
    "OpendTect",
    "Wayne Mogg",
    "1.0",
    "A starting point for writing an OpendTect attribute") );
    return &retpi;
}


mDefODInitPlugin(MT_MAI_MAO)
{
    Attrib::MT_MAI_MAO::initClass();
    return 0;
}
