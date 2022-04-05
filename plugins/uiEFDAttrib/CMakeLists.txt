#
#   uiEFDAttrib Plugin
#   Copyright (C) 2021  Wayne Mogg
#

SET(OD_MODULE_NAME uiEFDAttrib)
SET(OD_IS_PLUGIN 1 )
SET(OD_MODULE_DEPS uiODMain EFDAttrib uiWGMHelp )
SET(OD_MODULE_SOURCES
	uiefdmodesattrib.cc
	uiefdspectrumattrib.cc
	uiefdattribpi.cc)
SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
