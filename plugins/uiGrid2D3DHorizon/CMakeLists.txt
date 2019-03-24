SET(OD_MODULE_DEPS uiODMain uiWGMHelp)

SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
    uigrid2d3dhorizonmainwin.cc
    uigrid2d3dhorizonpi.cc
    uiinputgrp.cc
    uigridgrp.cc
    uiseis2dlineselgrp.cc
    wmgridder2d.cc )

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
