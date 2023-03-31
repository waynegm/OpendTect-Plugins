#pragma once

#include "wmplugins_version.h"
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

class wmPlugins
{
public:
    static const char* sKeyWMPlugins()	{ return "WMPlugins"; }
    static const char* sKeyWMSeismicSolutions()	{ return "WM Seismic Solutions"; }
    static const char* sKeyWMPluginsAuthor()	{ return "Wayne Mogg (WM Seismic Solutions)"; }
    static const char* sKeyWMPluginsVersion()	{ return STR(WMPLUGINS_VERSION); }

};
