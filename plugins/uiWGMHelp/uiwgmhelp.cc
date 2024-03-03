/*
 *   uiWGMHelp Plugin - help system and helper classes
 *  Copyright (C) 2019  Wayne Mogg
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "uiwgmhelp.h"

#include "bufstring.h"
#include "envvars.h"
#include "file.h"
#include "filepath.h"
#include "oddirs.h"
#include "pythonaccess.h"
#include "settings.h"

BufferString uiWGMHelp::GetIconFileName( const char* fnm, bool prefUser )
{
    FilePath fp;
    if ( prefUser ) {
	fp.set( GetEnvVar("OD_USER_PLUGIN_DIR") );
	fp.add( "data" ).add( "icons.Default" ).add( fnm );
	if ( fp.exists() )
	    return fp.fullPath();
    }
    fp.set( GetSoftwareDir( false ) );
    fp.add( "data" ).add( "icons.Default" ).add( fnm );
    ErrMsg(fp.fullPath());
    if ( fp.exists() )
	return fp.fullPath();
    else
	return BufferString::empty();
}

FilePath uiWGMHelp::GetPythonInterpPath()
{
    BufferString pythonstr( sKey::Python() ); pythonstr.toLower();
    FilePath fp;
    OD::PythA().GetPythonEnvPath( fp );
    if (File::isDirectory(fp.fullPath()) && File::isDirEmpty(fp.fullPath()))
	fp = FilePath( fp.dirUpTo(fp.nrLevels() - 2));

#ifdef __win__
    pythonstr.add(".exe");
#else
    fp.add("bin");
#endif
    
    fp.add( pythonstr );
    return fp.exists() ? fp : FilePath();
}
