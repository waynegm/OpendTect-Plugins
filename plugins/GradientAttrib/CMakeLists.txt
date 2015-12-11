#_______________________Pmake___________________________________________________
#
#	Makefile : 	GradientAttrib plugin
# 	Nov 2015	Wayne Mogg
#_______________________________________________________________________________

SET(OD_MODULE_DEPS AttributeEngine)
SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
	gradientattribpi.cc
	gradientattrib.cc)
SET( OD_PLUGIN_ALO_EXEC ${OD_ATTRIB_EXECS} )
OD_INIT_MODULE()
