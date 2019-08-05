#
#   LocalAttrib Plugin
#   Copyright (C) 2019  Wayne Mogg
#

SET(OD_MODULE_DEPS AttributeEngine)
SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
	ltfattrib.cc
	localattribpi.cc
	rsflib.cc
)

  add_definitions(-DNO_BLAS)

SET( OD_PLUGIN_ALO_EXEC ${OD_ATTRIB_EXECS} )
OD_INIT_MODULE()
