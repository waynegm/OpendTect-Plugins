#include "uimenu.h"
#include "uiodmain.h"
#include "odplugin.h"

#include "uiavopolarattrib.h"

mDefODPluginInfo(uiAVOPolarAttrib)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "AVO Polarization Attributes (UI)",
    "OpendTect",
    "Wayne Mogg",
    "1.0",
    "") );
    return &retpi;
}

mDefODInitPlugin(uiAVOPolarAttrib)
{
    uiAVOPolarAttrib::initClass();

    return 0;
}
