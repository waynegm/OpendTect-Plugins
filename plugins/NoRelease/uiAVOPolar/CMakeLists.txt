find_package(ArrayFire)
set(OD_MODULE_NAME uiAVOPolat)
set(OD_IS_PLUGIN yes )
set(OD_MODULE_DEPS  AVOPolar uiODMain)
set(OD_MODULE_SOURCES
	uiavopolar.cc
	uiavopolarpi.cc)
find_package(ArrayFire)
list( APPEND OD_MODULE_INCLUDESYSPATH ${ArrayFire_INCLUDE_DIRS} )
list( APPEND OD_MODULE_EXTERNAL_LIBS ${ArrayFire_Unified_LIBRARIES} )
SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
