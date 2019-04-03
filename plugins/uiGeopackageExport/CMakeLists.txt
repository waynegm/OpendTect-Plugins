list( APPEND OD_MODULE_INCLUDESYSPATH ${OpendTect_DIR}/plugins/CRS )

if (UNIX)
    list( APPEND OD_MODULE_EXTERNAL_LIBS ${OpendTect_DIR}/${OD_EXEC_OUTPUT_RELPATH}/libCRS.so )
    SET(CMAKE_INSTALL_RPATH "\$ORIGIN/")
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
else()
    list( APPEND OD_MODULE_EXTERNAL_LIBS ${OpendTect_DIR}/${OD_EXEC_OUTPUT_RELPATH}/Release/CRS.lib )
endif()
SET(OD_MODULE_DEPS uiODMain uiWGMHelp)

SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
    uigeopackageexportmainwin.cc
    uigeopackageexportpi.cc
    uigeopackagewriter.cc 
    uisurveygrp.cc 
    ui2dlinesgrp.cc 
    uirandomlinesgrp.cc 
    uiwellsgrp.cc 
    uipolylinesgrp.cc 
    uihorizongrp.cc )

find_package(GDAL)
list( APPEND OD_MODULE_INCLUDESYSPATH ${GDAL_INCLUDE_DIR} )
list( APPEND OD_MODULE_EXTERNAL_LIBS ${GDAL_LIBRARY} )

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
