SET(OD_MODULE_DEPS uiODMain CRS)
SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
    uigeopackageexportmainwin.cc
    uigeopackageexportpi.cc
    uigeopackagewriter.cc 
    uisurveygrp.cc 
    ui2dlinesgrp.cc 
    uirandomlinesgrp.cc 
    uiwellsgrp.cc 
    uipolylinesgrp.cc )

find_package(GDAL)
list( APPEND OD_MODULE_INCLUDESYSPATH ${GDAL_INCLUDE_DIR} )
list( APPEND OD_MODULE_EXTERNAL_LIBS ${GDAL_LIBRARY} )

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
