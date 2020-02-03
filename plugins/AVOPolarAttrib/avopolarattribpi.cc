#include "avopolarattrib.h"
#include "odplugin.h"
#include "avopolarattribmod.h"
#include "wmplugins.h"

mDefODPluginEarlyLoad(AVOPolarAttrib)
mDefODPluginInfo(AVOPolarAttrib)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "AVO Polarization Attributes (Base)",
    wmPlugins::sKeyWMPlugins(),
    wmPlugins::sKeyWMPluginsAuthor(),
    wmPlugins::sKeyWMPluginsVersion(),
    "AVO Polarization attributes for OpendTect") );
    return &retpi;
}


mDefODInitPlugin(AVOPolarAttrib)
{
    Attrib::AVOPolarAttrib::initClass();
    return 0;
}
