#include "st_mai_sao.h"
#include "odplugin.h"
#include "st_mai_saomod.h"


mDefODPluginEarlyLoad(ST_MAI_SAO)
mDefODPluginInfo(ST_MAI_SAO)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "Attribute skeleton for single-trace, multi-attribute input and single-attribute output",
    "OpendTect",
    "Wayne Mogg",
    "1.0",
    "A starting point for writing an OpendTect attribute") );
    return &retpi;
}


mDefODInitPlugin(ST_MAI_SAO)
{
    Attrib::ST_MAI_SAO::initClass();
    return 0;
}
