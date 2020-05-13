set(OD_MODULE_DEPS uiODMain)
set(OD_IS_PLUGIN yes )
set(OD_MODULE_SOURCES
	uiwgmhelppi.cc
	uiwgmhelp.cc
	ui3drangegrp.cc 
	uiseis2dlineselgrp.cc 
	uipolygonparsel.cc 
	uibufferstringsetselgrp.cc 
	uihorinputgrp.cc 
	ui2d3ddataselgrp.cc )
SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
