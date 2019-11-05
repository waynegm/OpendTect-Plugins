set(OD_MODULE_NAME uiAVOPolarAttrib)
set(OD_IS_PLUGIN yes )
set(OD_MODULE_DEPS uiODMain uiWGMHelp AVOPolarAttrib)
set(OD_MODULE_SOURCES
	uiavopolarattribpi.cc
	uiavopolarattrib.cc)
add_definitions( -DNDEBUG )

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
