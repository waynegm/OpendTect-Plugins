#
#   VMDAttrib Plugin
#   Copyright (C) 2021  Wayne Mogg
#

SET(OD_MODULE_DEPS AttributeEngine EigenTools)
SET(OD_IS_PLUGIN yes)
SET(OD_TEST_PROGS vmd_test.cc)

SET(OD_MODULE_SOURCES
	vmdspectrumattrib.cc
	vmdattribpi.cc
)

SET( OD_PLUGIN_ALO_EXEC ${OD_ATTRIB_EXECS} )
OD_INIT_MODULE()
