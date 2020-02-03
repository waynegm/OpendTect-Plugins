#pragma once

#include "version.h"

class wmPlugins
{
public:
    static const char* sKeyWMPlugins()	{ return "WMPlugins"; }
    static const char* sKeyWMSeismicSolutions()	{ return "WM Seismic Solutions"; }
    static const char* sKeyWMPluginsAuthor()	{ return "Wayne Mogg (WM Seismic Solutions)"; }
    static const char* sKeyWMPluginsVersion()	{ return wm_version; }
    
};
