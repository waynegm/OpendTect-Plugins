#include "avopolar.h"
#include "odplugin.h"
#include "avopolarmod.h"


mDefODPluginEarlyLoad(AVOPolar)
mDefODPluginInfo(AVOPolar)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "AVO Polarization attribute",
    "OpendTect",
    "Wayne Mogg",
    "1.0",
    "") );
    return &retpi;
}


mDefODInitPlugin(AVOPolar)
{
    Attrib::AVOPolar::initClass();
    return 0;
}
