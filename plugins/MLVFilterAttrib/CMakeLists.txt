#_______________________Pmake___________________________________________________
#
#	Makefile : 	MLVFilterAttrib plugin
# 	Jan 2014	Wayne Mogg
#_______________________________________________________________________________

SET(OD_MODULE_DEPS AttributeEngine)
SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
	mlvfilterattribpi.cc
	mlvfilterattrib.cc)
SET( OD_PLUGIN_ALO_EXEC ${OD_ATTRIB_EXECS} )
OD_INIT_MODULE()
