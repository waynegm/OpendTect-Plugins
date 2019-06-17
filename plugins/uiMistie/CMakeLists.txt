set(OD_MODULE_NAME uiMistie)
set(OD_IS_PLUGIN yes )
set(OD_MODULE_DEPS  Mistie uiODMain uiWGMHelp)
set(OD_MODULE_SOURCES
    uimistieestimatemainwin.cc
    uimistieanalysismainwin.cc
    uimistiecorrmainwin.cc
    uicorrviewer.cc
	uimistieapplier.cc
	uimistiepi.cc)
	

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
