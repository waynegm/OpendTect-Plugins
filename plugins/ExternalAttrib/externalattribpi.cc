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
 Date:          December 2014
 ________________________________________________________________________

-*/

#include "externalattrib.h"
#include "odplugin.h"
#include "msgh.h"
#include "wmplugins.h"

mDefODPluginEarlyLoad(ExternalAttrib)

mDefODPluginInfo(ExternalAttrib)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
	"External Attribute (Base)",
	wmPlugins::sKeyWMPlugins(),
	"Wayne Mogg (WM Seismic Solutions)",
	"=od",
	"Attribute calculation using an external process for OpendTect" ) );
    return &retpi;
}


mDefODInitPlugin(ExternalAttrib)
{
    Attrib::ExternalAttrib::initClass();
    return 0;
}

