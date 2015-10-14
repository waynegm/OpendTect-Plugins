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
 Date:          January 2014
 ________________________________________________________________________

-*/

#include "mlvfilterattrib.h"
#include "odplugin.h"
#include "mlvfilterattribmod.h"



mDefODPluginEarlyLoad(MLVFilterAttrib)

mDefODPluginInfo(MLVFilterAttrib)
{
	mDefineStaticLocalObject( PluginInfo, retpi,(
		"Mean of Least Variance Filter Attribute (base)",
		"Mean of Least Variance Filter Attribute (base)",
		"Wayne Mogg",
		"6.0",
    	"Mean of least variance structure preserving smoothing for OpendTect v5+",
		PluginInfo::GPL ) );
    return &retpi;
}


mDefODInitPlugin(MLVFilterAttrib)
{
    Attrib::MLVFilter::initClass();
    return 0;
}

