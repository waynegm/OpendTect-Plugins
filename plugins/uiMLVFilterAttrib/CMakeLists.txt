#_______________________Pmake___________________________________________________
#
#	Makefile : 	MLVFilterAttrib plugin
# 	Jan 2014	Wayne Mogg
#_______________________________________________________________________________

SET(OD_MODULE_DEPS MLVFilterAttrib uiODMain uiWGMHelp )
SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
	uimlvfilterattribpi.cc
	uimlvfilterattrib.cc)
SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
