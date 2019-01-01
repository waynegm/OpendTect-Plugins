#include "mt_mai_sao.h"
#include "odplugin.h"
#include "mt_mai_saomod.h"


mDefODPluginEarlyLoad(MT_MAI_SAO)
mDefODPluginInfo(MT_MAI_SAO)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "Attribute skeleton for multi-trace, multi-attribute input and single-attribute output",
    "OpendTect",
    "Wayne Mogg",
    "1.0",
    "A starting point for writing an OpendTect attribute") );
    return &retpi;
}


mDefODInitPlugin(MT_MAI_SAO)
{
    Attrib::MT_MAI_SAO::initClass();
    return 0;
}
