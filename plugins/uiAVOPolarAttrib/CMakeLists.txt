set(OD_MODULE_NAME uiAVOPolarAttrib)
set(OD_IS_PLUGIN yes )
set(OD_MODULE_DEPS uiODMain uiWGMHelp AVOPolarAttrib)
set(OD_MODULE_SOURCES
	uiavopolarattribpi.cc
	uiavopolarattrib.cc)
find_package(Eigen3)
list( APPEND OD_MODULE_INCLUDESYSPATH ${EIGEN3_INCLUDE_DIR} )
add_definitions( -DNDEBUG )

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
