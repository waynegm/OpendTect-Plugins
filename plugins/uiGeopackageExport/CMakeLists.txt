if ( IS_DIRECTORY "${WMDEPS_DIR}" )
    if ( NOT IS_DIRECTORY "${Tiff_ROOT}" )
	set( Tiff_ROOT "${WMDEPS_DIR}" )
    endif()
    if ( NOT IS_DIRECTORY "${GeoTIFF_ROOT}" )
	set( GeoTIFF_ROOT "${WMDEPS_DIR}" )
    endif()
    if ( NOT IS_DIRECTORY "${gpkg_ROOT}" )
	set( gpkg_ROOT "${WMDEPS_DIR}" )
    endif()
    if ( NOT IS_DIRECTORY "${gpkgio_ROOT}" )
	set( gpkgio_ROOT "${WMDEPS_DIR}" )
    endif()
endif()

find_package( SQLite3 QUIET GLOBAL )
if ( NOT SQLite3_FOUND )
    find_package( SQLite3 QUIET REQUIRED )
endif()
find_package( GeoTIFF QUIET GLOBAL )
if ( NOT GeoTIFF_FOUND )
    find_package( GeoTIFF REQUIRED )
endif()
find_package( gpkg QUIET GLOBAL )
if ( NOT gpkg_FOUND )
    find_package( gpkg REQUIRED GLOBAL )
endif()
find_package( gpkgio QUIET GLOBAL )
if ( NOT gpkgio_FOUND )
    find_package( gpkgio REQUIRED GLOBAL )
endif()

list( APPEND OD_MODULE_EXTERNAL_LIBS
    TIFF::tiff
    SQLite::SQLite3
    ${GeoTIFF_SHARED_LIBRARIES}
    gpkgio::gpkg_ext
    gpkgio::gpkgio
)

SET(OD_MODULE_DEPS uiODMain uiWGMHelp CRS)

SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
#    uigeopackagereader.cc
#    uigeopackagetreeitem.cc
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
    uihorizongrp.cc
)

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
