#include "avopolarattrib.h"
#include "odplugin.h"
#include "avopolarattribmod.h"


mDefODPluginEarlyLoad(AVOPolarAttrib)
mDefODPluginInfo(AVOPolarAttrib)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "AVO Polarization Attributes",
    "OpendTect",
    "Wayne Mogg",
    "1.0",
    "") );
    return &retpi;
}


mDefODInitPlugin(AVOPolarAttrib)
{
    Attrib::AVOPolarAttrib::initClass();
    return 0;
}
