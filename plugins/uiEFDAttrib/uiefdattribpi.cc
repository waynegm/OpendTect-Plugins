/*Copyright (C) 2021 Wayne Mogg. All rights reserved.
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
 Date:		September 2021
 ________________________________________________________________________

 -*/

#include "uimenu.h"
#include "uiodmain.h"
#include "odplugin.h"

#include "uiefdmodesattrib.h"
#include "uiefdspectrumattrib.h"
#include "wmplugins.h"

mDefODPluginInfo(uiEFDAttrib)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
	"Empirical Fourier Decomposition Attributes (UI)",
	wmPlugins::sKeyWMPlugins(),
	wmPlugins::sKeyWMPluginsAuthor(),
	wmPlugins::sKeyWMPluginsVersion(),
	"Empirical Fourier Decomposition attribute for OpendTect" ) );
    return &retpi;
}

mDefODInitPlugin(uiEFDAttrib)
{
    uiEFDSpectrumAttrib::initClass();
    uiEFDModesAttrib::initClass();

    return 0;
}
