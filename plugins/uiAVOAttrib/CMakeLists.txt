#_______________________Pmake___________________________________________________
#
#	Makefile : 	AVO Attribute
# 	February 2014	Wayne Mogg
#_______________________________________________________________________________

SET(OD_MODULE_NAME uiAVOAttrib)
SET(OD_IS_PLUGIN 1 )
SET(OD_MODULE_DEPS uiODMain AVOAttrib)
SET(OD_MODULE_SOURCES 
	uiavoattrib.cc
	uiavoattribpi.cc)
SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
