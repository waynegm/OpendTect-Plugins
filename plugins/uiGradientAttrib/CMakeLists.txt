#_______________________Pmake___________________________________________________
#
#	Makefile : 	uiGradientAttrib plugin
# 	Nov 2015	Wayne Mogg
#_______________________________________________________________________________

SET(OD_MODULE_DEPS GradientAttrib uiODMain uiWGMHelp )
SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
	uigradientattribpi.cc
	uigradientattrib.cc)
SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
