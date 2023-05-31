if (UNIX)
    SET(CMAKE_INSTALL_RPATH "\$ORIGIN/")
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

list( APPEND OD_MODULE_INCLUDESYSPATH
    ${OpendTect_DIR}/plugins/CRS
    ${SQLITE_DIR}/include
    ${WMDEPS_DIR}/include
)

find_library(
    TIFF_LIBRARY
    NAMES tiff
    PATHS ${WMDEPS_DIR}/lib
    NO_DEFAULT_PATH
)
find_library(
    GEOTIFF_LIBRARY
    NAMES geotiff geotiff_i
    PATHS ${WMDEPS_DIR}/lib
    NO_DEFAULT_PATH
)
find_library(
    GPKGIO_LIBRARY
    NAMES gpkgio
    PATHS ${WMDEPS_DIR}/lib
    NO_DEFAULT_PATH
)
find_library(
    GPKGEXT_LIBRARY
    NAMES gpkg
    PATHS ${WMDEPS_DIR}/lib
    NO_DEFAULT_PATH
)


list( APPEND OD_MODULE_EXTERNAL_LIBS
    ${TIFF_LIBRARY}
    ${GEOTIFF_LIBRARY}
    ${GPKGIO_LIBRARY}
    ${GPKGEXT_LIBRARY}
    ${SQLITE_LIBRARY}
)
string(REPLACE ";"  "\n " stri "${OD_MODULE_INCLUDESYSPATH}")
message("${stri}")

string(REPLACE ";"  "\n " str "${OD_MODULE_EXTERNAL_LIBS}")
message("${str}")

SET(OD_MODULE_DEPS uiODMain uiWGMHelp CRS)

SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
#uigeopackagereader.cc
#uigeopackagetreeitem.cc
    uigeotiffexportmainwin.cc
    uigeotiffwriter.cc
    uigeopackageexportmainwin.cc
    uigeopackageexportpi.cc
    uigeopackagewriter.cc
    uisurveygrp.cc
    ui2dlinesgrp.cc
    uirandomlinesgrp.cc
    uiwellsgrp.cc
    uipolylinesgrp.cc
    uihorizongrp.cc )

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
