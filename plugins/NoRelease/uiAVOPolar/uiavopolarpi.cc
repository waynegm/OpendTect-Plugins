#include "uimenu.h"
#include "uiodmain.h"
#include "odplugin.h"

#include "uiavopolar.h"

mDefODPluginInfo(uiAVOPolar)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "AVO Polarization attribute (UI)",
    "OpendTect",
    "Wayne Mogg",
    "1.0",
    "") );
    return &retpi;
}

mDefODInitPlugin(uiAVOPolar)
{
    uiAVOPolar::initClass();

    return 0;
}
