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
 Date:          April 2014
 ________________________________________________________________________
 
 -*/

#include "ltfattrib.h"
#include "odplugin.h"


mDefODPluginEarlyLoad(LTFAttrib)
mDefODPluginInfo(LTFAttrib)
{
	mDefineStaticLocalObject( PluginInfo, retpi,(
		"Local Time-Frequency Attribute (base)",
		"Local Time-Frequency Attribute (base)",
		"Wayne Mogg",
		"6.0",
		"Time-Frequency decomposition using local attibutes for OpendTect",
		PluginInfo::GPL ) );
	return &retpi;
}


mDefODInitPlugin(LTFAttrib)
{
    Attrib::LTFAttrib::initClass();

    return 0;
}

