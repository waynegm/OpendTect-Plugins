#_______________________Pmake___________________________________________________
#
#	Makefile : 	ExternalAttrib plugin
# 	Dec 2014	Wayne Mogg
#_______________________________________________________________________________

SET(OD_MODULE_DEPS AttributeEngine uiWGMHelp)

SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
	extproc.cc
	procinst.cc
	externalattribpi.cc
	externalattrib.cc)
SET( OD_PLUGIN_ALO_EXEC ${OD_ATTRIB_EXECS} )
list(APPEND CMAKE_MODULE_PATH "CMakeModules")
OD_INIT_MODULE()
