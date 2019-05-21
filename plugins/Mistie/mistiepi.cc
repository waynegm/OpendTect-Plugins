#include "mistieapplier.h"
#include "odplugin.h"
#include "mistiemod.h"


mDefODPluginEarlyLoad(Mistie)
mDefODPluginInfo(Mistie)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "Mistie analysis and correction plugin base",
    "OpendTect",
    "Wayne Mogg",
    "1.0",
    "Back-end for the plugin for Mistie analysis and correction") );
    return &retpi;
}


mDefODInitPlugin(Mistie)
{
    Attrib::MistieApplier::initClass();
    
    return 0;
}
