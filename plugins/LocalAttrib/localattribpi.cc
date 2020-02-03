/*
 *   LocalAttrib Plugin
 *   Copyright (C) 2019  Wayne Mogg
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ltfattrib.h"
#include "odplugin.h"

#include "wmplugins.h"

mDefODPluginEarlyLoad(LocalAttrib)
mDefODPluginInfo(LocalAttrib)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
	"Local seismic attributes (Base)",
	wmPlugins::sKeyWMPlugins(),
	wmPlugins::sKeyWMPluginsAuthor(),
	wmPlugins::sKeyWMPluginsVersion(),
	"Various local seismic attributes as per Madagascar but without the dependencies" ) );
    return &retpi;
}


mDefODInitPlugin(LocalAttrib)
{
    Attrib::LTFAttrib::initClass();

    return 0;
}

