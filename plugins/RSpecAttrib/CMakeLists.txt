#_______________________Pmake___________________________________________________
#
#	Makefile : 	Recursive Spectral Decomposition Attribute
# 	August 2014	Wayne Mogg
#_______________________________________________________________________________

SET(OD_MODULE_DEPS EigenTools AttributeEngine)
SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
	rspecattrib.cc
	rspecattribpi.cc
)

add_definitions( -DNDEBUG )

SET( OD_PLUGIN_ALO_EXEC ${OD_ATTRIB_EXECS} )
OD_INIT_MODULE()
