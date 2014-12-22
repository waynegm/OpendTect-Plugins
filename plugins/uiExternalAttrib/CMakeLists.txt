#_______________________Pmake___________________________________________________
#
#	Makefile : 	External Attribute
# 	Dec 2014	Wayne Mogg
#_______________________________________________________________________________

SET(OD_MODULE_DEPS ExternalAttrib uiODMain)
SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
	uiexternalattribpi.cc
	uiexternalattrib.cc)
SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
