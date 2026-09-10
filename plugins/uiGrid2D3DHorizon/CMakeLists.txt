if ( POLICY CMP0169 )
    cmake_policy( SET CMP0169 OLD )
endif()
include(FetchContent)
FetchContent_Declare(
    mba_lib
    GIT_REPOSITORY https://github.com/ddemidov/mba
    GIT_TAG        0.3
)
FetchContent_GetProperties(mba_lib)
if(NOT mba_lib_POPULATED)
    FetchContent_Populate(mba_lib)
endif()

include_directories(SYSTEM "${mba_lib_SOURCE_DIR}")

SET(OD_MODULE_DEPS uiODMain uiWGMHelp)

SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
    wmgridder2d.cc
    ltpsgridder2d.cc
    mbagridder2d.cc
    idwgridder2d.cc
    nrngridder2d.cc
    uigrid2d3dhorizonmainwin.cc
    uigrid2d3dhorizonpi.cc
    uiinputgrp.cc
    uigridgrp.cc )
    
add_definitions( -DNDEBUG )

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
