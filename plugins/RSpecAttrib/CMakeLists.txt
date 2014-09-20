#_______________________Pmake___________________________________________________
#
#	Makefile : 	Recursive Spectral Decomposition Attribute
# 	August 2014	Wayne Mogg
#_______________________________________________________________________________

SET(OD_MODULE_DEPS AttributeEngine)
SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
	rspecattrib.cc
	rspecattribpi.cc
)
SET( OD_PLUGIN_ALO_EXEC ${OD_ATTRIB_EXECS} )
OD_INIT_MODULE()
