
find_package( SQLite3 QUIET GLOBAL )
if ( NOT TARGET SQLite::SQLite3 )
    set( SQLite3_ROOT "" CACHE PATH "Sqlite directory" )
    find_package( SQLite3 QUIET REQUIRED )
endif()
if ( TARGET SQLite::SQLite3 AND UNIX AND NOT APPLE )
    # FindSQLite3 leaves IMPORTED_SONAME unset, so OD's runtime-lib staging can't
    # resolve sqlite3's .so.0. Its soname is stable across Linux distros and conda.
    set_target_properties( SQLite::SQLite3 PROPERTIES IMPORTED_SONAME "libsqlite3.so.0" )
endif()
find_package( Tiff QUIET GLOBAL )
if ( NOT TARGET TIFF::tiff )
    set( Tiff_ROOT "" CACHE PATH "Tiff directory" )
    find_package( Tiff REQUIRED GLOBAL )
endif()
find_package( GeoTIFF QUIET GLOBAL )
if ( NOT GeoTIFF_FOUND )
    set( GeoTIFF_ROOT "" CACHE PATH "GeoTIFF directory" )
    find_package( GeoTIFF REQUIRED )
endif()
find_package( gpkg QUIET GLOBAL )
if ( NOT TARGET gpkgio::gpkg_ext )
    set( gpkg_ROOT "" CACHE PATH "gpkg directory" )
    find_package( gpkg REQUIRED GLOBAL )
endif()
find_package( gpkgio QUIET GLOBAL )
if ( NOT TARGET gpkgio::gpkgio )
    set( gpkgio_ROOT "" CACHE PATH "gpkg directory" )
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