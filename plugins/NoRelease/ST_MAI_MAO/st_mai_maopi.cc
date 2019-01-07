#include "st_mai_mao.h"
#include "odplugin.h"
#include "st_mai_maomod.h"


mDefODPluginEarlyLoad(ST_MAI_MAO)
mDefODPluginInfo(ST_MAI_MAO)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "Attribute skeleton for single-trace, multi-attribute input and multi-attribute output",
    "OpendTect",
    "Wayne Mogg",
    "1.0",
    "A starting point for writing an OpendTect attribute") );
    return &retpi;
}


mDefODInitPlugin(ST_MAI_MAO)
{
    Attrib::ST_MAI_MAO::initClass();
    return 0;
}
