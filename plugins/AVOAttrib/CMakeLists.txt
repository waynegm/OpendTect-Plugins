#_______________________Pmake___________________________________________________
#
#	Makefile : 	AVO Attribute
# 	February 2014	Wayne Mogg
#_______________________________________________________________________________

SET(OD_MODULE_DEPS AttributeEngine EarthModel Well)
SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
	avoattrib.cc
	avoattribpi.cc)
SET( OD_PLUGIN_ALO_EXEC ${OD_ATTRIB_EXECS} )
OD_INIT_MODULE()
