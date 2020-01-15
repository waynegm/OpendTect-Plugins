#include "mistieapplier.h"
#include "odplugin.h"
#include "mistiemod.h"
#include "wmplugins.h"

mDefODPluginEarlyLoad(Mistie)
mDefODPluginInfo(Mistie)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "Mistie analysis and correction (Base)",
    wmPlugins::sKeyWMPlugins(),
    "Wayne Mogg (WM Seismic Solutions)",
    "=od",
    "Mistie analysis and correction for OpendTect") );
    return &retpi;
}


mDefODInitPlugin(Mistie)
{
    Attrib::MistieApplier::initClass();
    
    return 0;
}
