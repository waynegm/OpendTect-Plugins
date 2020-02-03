/*Copyright (C) 2014 Wayne Mogg All rights reserved.
  
 This file may be used either under the terms of:
 
 1. The GNU General Public License version 3 or higher, as published by
 the Free Software Foundation, or
 
 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*+
 ________________________________________________________________________

 Author:        Wayne Mogg
 Date:          August 2014
 ________________________________________________________________________
 
 -*/

#include "rspecattrib.h"
#include "odplugin.h"
#include "wmplugins.h"

mDefODPluginEarlyLoad(RSpecAttrib)
mDefODPluginInfo(RSpecAttrib)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
	"Recursive spectral decomposition attribute (Base)",
	wmPlugins::sKeyWMPlugins(),
	wmPlugins::sKeyWMPluginsAuthor(),
	wmPlugins::sKeyWMPluginsVersion(),
	"Recursive spectral decomposition attribute for OpendTect" ) );
    return &retpi;
}


mDefODInitPlugin(RSpecAttrib)
{
    Attrib::RSpecAttrib::initClass();

    return 0;
}

