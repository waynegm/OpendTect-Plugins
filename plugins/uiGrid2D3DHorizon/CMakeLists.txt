SET(OD_MODULE_DEPS uiODMain uiWGMHelp)

SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
    wmgridder2d.cc
    mbagridder2d.cc
    rbfgridder2d.cc
    idwgridder2d.cc
    uigrid2d3dhorizonmainwin.cc
    uigrid2d3dhorizonpi.cc
    uiinputgrp.cc
    uigridgrp.cc )
    
add_definitions( -DNDEBUG )

list( APPEND OD_MODULE_INCLUDESYSPATH ${CMAKE_SOURCE_DIR}/plugins/external/include )
list( APPEND OD_MODULE_EXTERNAL_LIBS ${CMAKE_SOURCE_DIR}/plugins/external/lib/libmathtoolbox.a )

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
