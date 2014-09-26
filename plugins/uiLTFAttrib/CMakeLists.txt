#_______________________Pmake___________________________________________________
#
#	Makefile : 	Local Time Frequency Attribute
# 	April 2014	Wayne Mogg
#_______________________________________________________________________________

SET(OD_MODULE_NAME uiLTFAttrib)
SET(OD_IS_PLUGIN 1 )
SET(OD_MODULE_DEPS uiODMain LTFAttrib)
SET(OD_MODULE_SOURCES 
	uiltfattrib.cc
	uiltfattribpi.cc)
SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
