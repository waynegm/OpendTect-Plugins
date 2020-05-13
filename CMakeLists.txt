#_______________________Pmake___________________________________________________
#
#	Makefile : 	Tutorial plugin
# 	Jan 2012	A.H. Bril
#	RCS :		$Id: CMakeLists.txt 29756 2013-05-13 11:31:35Z ranojay.sen@dgbes.com $
#_______________________________________________________________________________


cmake_minimum_required (VERSION 3.0)
project(OpendTect_Plugins)
set ( OD_SUBSYSTEM "waynegm_plugin" )
set ( OpendTect_DIR "" CACHE PATH "OpendTect directory" )

add_subdirectory(external)
include_directories(external/include plugins/wm_include)

#Setup output directory
option ( OD_BUILD_LOCAL "Build in local directory" ON )

#Find OpendTect
list(APPEND CMAKE_MODULE_PATH "${OpendTect_DIR}/CMakeModules")
find_package( OpendTect )

#Setup the modules
set ( PLUGINS uiWGMHelp
            uiWMTools
            uiGeopackageExport
            uiGrid2D3DHorizon
            Mistie uiMistie
            AVOPolarAttrib uiAVOPolarAttrib
            RSpecAttrib uiRSpecAttrib 
            MLVFilterAttrib  uiMLVFilterAttrib 
            AVOAttrib uiAVOAttrib
            ExternalAttrib uiExternalAttrib
            GradientAttrib uiGradientAttrib
            LocalAttrib uiLocalAttrib )
OD_ADD_MODULES( plugins ${PLUGINS} )

#Add the plugins
foreach ( plugin ${PLUGINS} )
    list( APPEND RUNTIMEPATH ${OD_${plugin}_RUNTIMEPATH} )
endforeach()

list ( APPEND RUNTIMEPATH ${OD_BINARY_BASEDIR}/${OD_EXEC_OUTPUT_RELPATH} )
list ( REMOVE_DUPLICATES RUNTIMEPATH )

if( OD_CREATE_LAUNCHERS )
    set ( OD_LAUNCHERS od_main od_process_attrib od_process_volume )
    foreach ( LAUNCHER ${OD_LAUNCHERS} )
	add_custom_target( ${LAUNCHER} ALL )
	foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
	    set_target_properties( ${LAUNCHER}
		    PROPERTIES
		    LOCATION_${CONFIG}
	${OpendTect_DIR}/${OD_EXEC_OUTPUT_RELPATH}/${CONFIG}/${LAUNCHER}${OD_EXECUTABLE_EXTENSION} )
	endforeach()

	create_target_launcher( ${LAUNCHER}
	    RUNTIME_LIBRARY_DIRS
	 ${OpendTect_DIR}/${OD_EXEC_OUTPUT_RELPATH}
	 ${OD_BINARY_BASEDIR}/${OD_EXEC_OUTPUT_RELPATH} 
	    ENVIRONMENT
	    DTECT_APPL=${OpendTect_DIR}
	    WORK=${OpendTect_DIR} )
    endforeach()
endif( OD_CREATE_LAUNCHERS )

OD_WRITE_ALOFILES( ${CMAKE_CURRENT_BINARY_DIR} )

install(FILES LICENSE.txt DESTINATION ${CMAKE_INSTALL_PREFIX} RENAME wmplugin_LICENSE.txt)
install(FILES CHANGES.MD DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/ReleaseInfo RENAME RELEASE.wmplugins.txt)
install(DIRECTORY data DESTINATION ${CMAKE_INSTALL_PREFIX})
if( ${OD_PLFSUBDIR} STREQUAL "win64" )
    install(FILES VERSION.txt DESTINATION ${CMAKE_INSTALL_PREFIX}/relinfo RENAME ver.wmplugins_win64.txt)
    install(DIRECTORY ${OpendTect_Plugins_BINARY_DIR}/bin/${OD_PLFSUBDIR} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/ )
elseif( ${OD_PLFSUBDIR} STREQUAL "lux64" )
    install(FILES VERSION.txt DESTINATION ${CMAKE_INSTALL_PREFIX}/relinfo RENAME ver.wmplugins_lux64.txt)
endif()
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/python DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/ PATTERN "__pycache__" EXCLUDE)

if(${CMAKE_SOURCE_DIR}/VERSION.txt IS_NEWER_THAN ${CMAKE_SOURCE_DIR}/plugins/wm_include/version.h)
    file(READ ${CMAKE_SOURCE_DIR}/VERSION.txt content)
    set(delim "===")
    set(content "static const char* wm_version = R\"${delim}(\n${content})${delim}\";")
    file(WRITE ${CMAKE_SOURCE_DIR}/plugins/wm_include/version.h "${content}")
endif()

