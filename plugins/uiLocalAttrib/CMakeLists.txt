#
#   uiLocalAttrib Plugin
#   Copyright (C) 2019  Wayne Mogg
#

SET(OD_MODULE_NAME uiLocalAttrib)
SET(OD_IS_PLUGIN 1 )
SET(OD_MODULE_DEPS uiODMain LocalAttrib uiWGMHelp )
SET(OD_MODULE_SOURCES 
	uiltfattrib.cc
	uilocalattribpi.cc)
SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
