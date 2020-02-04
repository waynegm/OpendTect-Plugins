#include "uimenu.h"
#include "uiodmain.h"
#include "odplugin.h"

#include "uiavopolarattrib.h"
#include "wmplugins.h"

mDefODPluginInfo(uiAVOPolarAttrib)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
	"AVO Polarization Attributes (UI)",
	wmPlugins::sKeyWMPlugins(),
	wmPlugins::sKeyWMPluginsAuthor(),
	wmPlugins::sKeyWMPluginsVersion(),
	"AVO Polarization attribute for OpendTect" ) );
    return &retpi;
}

mDefODInitPlugin(uiAVOPolarAttrib)
{
    uiAVOPolarAttrib::initClass();

    return 0;
}
