#_______________________Pmake___________________________________________________
#
#	Makefile : 	Recursive Spectral Decomposition Attribute
# 	August 2014	Wayne Mogg
#_______________________________________________________________________________

SET(OD_MODULE_NAME uiRSpecAttrib)
SET(OD_IS_PLUGIN 1 )
SET(OD_MODULE_DEPS uiODMain RSpecAttrib)
SET(OD_MODULE_SOURCES 
	uirspecattrib.cc
	uirspecattribpi.cc)
SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
