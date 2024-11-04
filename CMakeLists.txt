#_______________________Pmake___________________________________________________
#
#	Makefile : 	Tutorial plugin
# 	Jan 2012	A.H. Bril
#	RCS :		$Id: CMakeLists.txt 29756 2013-05-13 11:31:35Z ranojay.sen@dgbes.com $
#_______________________________________________________________________________


cmake_minimum_required (VERSION 3.20)
project(OpendTect_Plugins VERSION 7.0.7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set ( OD_SUBSYSTEM "waynegm_plugin" )
set ( OpendTect_DIR "" CACHE PATH "OpendTect directory" )
configure_file(wmplugins_version.h.in wmplugins_version.h)
file (WRITE "${CMAKE_SOURCE_DIR}/VERSION.txt" ${CMAKE_PROJECT_VERSION})

if( NOT IS_DIRECTORY "${EIGEN3_INCLUDE_DIR}" )
    message("Unable to find system Eigen install - will use local")
    execute_process(COMMAND git submodule update --init -- external/eigen
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    file( COPY ${CMAKE_SOURCE_DIR}/external/eigen/Eigen DESTINATION ${CMAKE_SOURCE_DIR}/external/include )
    file( COPY ${CMAKE_SOURCE_DIR}/external/eigen/unsupported/Eigen DESTINATION ${CMAKE_SOURCE_DIR}/external/include/unsupported )
    set(EIGEN3_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/include)
endif()

if( NOT IS_DIRECTORY "${NANOFLANN_INCLUDE_DIR}" )
    message("Unable to find system nanoflann install - will use local")
    execute_process(COMMAND git submodule update --init -- external/nanoflann
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    file( COPY ${CMAKE_SOURCE_DIR}/external/nanoflann/include/nanoflann.hpp DESTINATION ${CMAKE_SOURCE_DIR}/external/include )
    file( RENAME ${CMAKE_SOURCE_DIR}/external/include/nanoflann.hpp ${CMAKE_SOURCE_DIR}/external/include/nanoflann.h )
    set(NANOFLANN_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/include)
endif()

if( NOT IS_DIRECTORY "${MBA_INCLUDE_DIR}" )
    message("Unable to find system mba install - will use local")
    execute_process(COMMAND git submodule update --init -- external/mba
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    file( COPY ${CMAKE_SOURCE_DIR}/external/mba/mba/mba.hpp DESTINATION ${CMAKE_SOURCE_DIR}/external/include )
    set(MBA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/include)
endif()

add_subdirectory(external)
include_directories(SYSTEM external/include plugins/wm_include "${PROJECT_BINARY_DIR}")

#Setup output directory
option ( OD_BUILD_LOCAL "Build in local directory" OFF )

#Find OpendTect
list(APPEND CMAKE_MODULE_PATH "${OpendTect_DIR}/CMakeModules")
find_package( OpendTect )
get_filename_component( OpendTect_DIR "${OpendTect_DIR}" REALPATH )
get_filename_component( OD_BINARY_BASEDIR "${OD_BINARY_BASEDIR}" REALPATH )

#Setup the modules
set ( PLUGINS uiWGMHelp
	    EigenTools
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
            LocalAttrib uiLocalAttrib
#            VMDAttrib uiVMDAttrib
            EFDAttrib uiEFDAttrib )
OD_ADD_MODULES( plugins ${PLUGINS} )

#Add the plugins
foreach ( plugin ${PLUGINS} )
    get_filename_component( OD_${plugin}_RUNTIMEPATH "${OD_${PLUGIN}_RUNTIMEPATH}" REALPATH CACHE )
    list( APPEND RUNTIMEPATH ${OD_${plugin}_RUNTIMEPATH} )
endforeach()

list ( APPEND RUNTIMEPATH ${OpendTect_DIR}/${OD_EXEC_OUTPUT_RELPATH} )
list ( APPEND RUNTIMEPATH ${OD_BINARY_BASEDIR}/${OD_EXEC_OUTPUT_RELPATH} )
list ( REMOVE_DUPLICATES RUNTIMEPATH )

if( OD_CREATE_LAUNCHERS )
	set ( OD_LAUNCHERS od_main od_process_attrib od_process_volume )
    set ( ENV{DTECT_BINDIR} "${OpendTect_DIR}/${OD_EXEC_OUTPUT_RELPATH}" )
    foreach ( LAUNCHER ${OD_LAUNCHERS} )
	add_custom_target( ${LAUNCHER} ALL )
	create_target_launcher( ${LAUNCHER}
	    COMMAND "${LAUNCHER}${OD_EXECUTABLE_EXTENSION}"
	    RUNTIME_LIBRARY_DIRS "${RUNTIMEPATH}"
	    ENVIRONMENT
	    DTECT_APPL=${OpendTect_DIR}
	    WORK=${OpendTect_DIR} )
    endforeach()
endif( OD_CREATE_LAUNCHERS )

OD_WRITE_ALOFILES( ${CMAKE_CURRENT_BINARY_DIR} )

install(FILES LICENSE.txt DESTINATION ${CMAKE_INSTALL_PREFIX} RENAME wmplugin_LICENSE.txt)
install(FILES CHANGES.MD DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/ReleaseInfo RENAME RELEASE.wmplugins.txt)
#install(DIRECTORY data DESTINATION ${CMAKE_INSTALL_PREFIX})
if( ${OD_PLFSUBDIR} STREQUAL "win64" )
    install(FILES VERSION.txt DESTINATION ${CMAKE_INSTALL_PREFIX}/relinfo RENAME ver.wmplugins_win64.txt)
    install(DIRECTORY ${OpendTect_Plugins_BINARY_DIR}/bin/${OD_PLFSUBDIR} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/ )
elseif( ${OD_PLFSUBDIR} STREQUAL "lux64" )
    install(FILES VERSION.txt DESTINATION ${CMAKE_INSTALL_PREFIX}/relinfo RENAME ver.wmplugins_lux64.txt)
endif()
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/python DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/ PATTERN "__pycache__" EXCLUDE)
