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
 Date:          April 2014
 ________________________________________________________________________
 
 -*/

#include "uimenu.h"
#include "uiodmain.h"
#include "odplugin.h"

#include "uiltfattrib.h"

mDefODPluginInfo(uiLTFAttrib)
{
	mDefineStaticLocalObject( PluginInfo, retpi,(
		"Time-Frequency analysis using local attributes v5 (UI)",
		"Time-Frequency analysis using local attributes v5 (UI)",
		"Wayne Mogg",
		"5.0",
		"",
		PluginInfo::GPL ) );
	return &retpi;
}

mDefODInitPlugin(uiLTFAttrib)
{
	uiLTFAttrib::initClass();
	return 0;
}

