SET(OD_MODULE_DEPS uiODMain uiWGMHelp)

SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
    uiwmtoolspi.cc
    uidehmainwin.cc
    uiconvexhull.cc
    uifaultpoly.cc
    uicontourpoly.cc )

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
