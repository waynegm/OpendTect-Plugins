#
#   EFDAttrib Plugin
#   Copyright (C) 2021  Wayne Mogg
#

SET(OD_MODULE_DEPS EigenTools AttributeEngine)
SET(OD_IS_PLUGIN yes)

SET(OD_MODULE_SOURCES
	efdspectrumattrib.cc
	efdattribpi.cc
)

SET( OD_PLUGIN_ALO_EXEC ${OD_ATTRIB_EXECS} )
OD_INIT_MODULE()
