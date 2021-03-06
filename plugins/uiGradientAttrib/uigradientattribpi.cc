/*Copyright (C) 2015 Wayne Mogg. All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          November 2015
 ________________________________________________________________________

-*/

#include "uimenu.h"
#include "uiodmain.h"
#include "odplugin.h"

#include "uigradientattrib.h"
#include "wmplugins.h"


mDefODPluginInfo(uiGradientAttrib)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
	"Gradient Attribute (UI)",
	wmPlugins::sKeyWMPlugins(),
	wmPlugins::sKeyWMPluginsAuthor(),
	wmPlugins::sKeyWMPluginsVersion(),
	"Inline, crossline and Z gradients for OpendTect" ) );
    return &retpi;
}


mDefODInitPlugin(uiGradientAttrib)
{
    uiGradientAttrib::initClass();
   return 0;
}

