SET(OD_MODULE_DEPS uiODMain uiWGMHelp)

SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
    wmgridder2d.cc
    uigrid2d3dhorizonmainwin.cc
    uigrid2d3dhorizonpi.cc
    uiinputgrp.cc
    uigridgrp.cc )
    
find_package(Eigen3)
list( APPEND OD_MODULE_INCLUDESYSPATH ${EIGEN3_INCLUDE_DIR} )
add_definitions( -DNDEBUG )

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
