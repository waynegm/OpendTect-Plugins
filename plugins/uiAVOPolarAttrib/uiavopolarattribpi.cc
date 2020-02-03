/*Copyright (C) 2014 Wayne Mogg. All rights reserved.
 *
 T hi*s file may be used either under the terms of:

 1. The GNU General Public License version 3 or higher, as published by
 the Free Software Foundation, or

 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*+
 _ __*_____________________________________________________________________

 Author:        Wayne Mogg
 Date:
 ________________________________________________________________________

 -*/

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
