if (UNIX)
    SET(CMAKE_INSTALL_RPATH "\$ORIGIN/")
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

list( APPEND OD_MODULE_INCLUDESYSPATH
    ${OpendTect_DIR}/plugins/CRS
    ${SQLite3_ROOT}/include
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
    NAMES geotiff geotiff_i geotiff_d
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

add_library(WMDEPS::tiff SHARED IMPORTED)
add_library(WMDEPS::geotiff SHARED IMPORTED)
add_library(WMDEPS::gpkgio SHARED IMPORTED)
add_library(WMDEPS::gpkgext SHARED IMPORTED)

# get_filename_component(TIFF_LIBRARY_RP ${TIFF_LIBRARY} REALPATH)
# get_filename_component(GEOTIFF_LIBRARY_RP ${GEOTIFF_LIBRARY} REALPATH)
# get_filename_component(GPKGIO_LIBRARY_RP ${GPKGIO_LIBRARY} REALPATH)
# get_filename_component(GPKGEXT_LIBRARY_RP ${GPKGEXT_LIBRARY} REALPATH)
# set_target_properties(WMDEPS::tiff PROPERTIES
#   IMPORTED_LOCATION ${TIFF_LIBRARY_RP}
# )
#
# set_target_properties(WMDEPS::geotiff PROPERTIES
#   IMPORTED_LOCATION ${GEOTIFF_LIBRARY_RP}
# )
#
# set_target_properties(WMDEPS::gpkgio PROPERTIES
#   IMPORTED_LOCATION ${GPKGIO_LIBRARY_RP}
#   IMPORTED_LINK_DEPENDENT_LIBRARIES SQLite::SQLite3
# )
#
# set_target_properties(WMDEPS::gpkgext PROPERTIES
#   IMPORTED_LOCATION ${GPKGEXT_LIBRARY_RP}
#   IMPORTED_LINK_DEPENDENT_LIBRARIES SQLite::SQLite3
# )
set_target_properties(WMDEPS::tiff PROPERTIES
  IMPORTED_LOCATION ${TIFF_LIBRARY}
)

set_target_properties(WMDEPS::geotiff PROPERTIES
  IMPORTED_LOCATION ${GEOTIFF_LIBRARY}
)

set_target_properties(WMDEPS::gpkgio PROPERTIES
  IMPORTED_LOCATION ${GPKGIO_LIBRARY}
  IMPORTED_LINK_DEPENDENT_LIBRARIES SQLite::SQLite3
)

set_target_properties(WMDEPS::gpkgext PROPERTIES
  IMPORTED_LOCATION ${GPKGEXT_LIBRARY}
  IMPORTED_LINK_DEPENDENT_LIBRARIES SQLite::SQLite3
)

list( APPEND OD_MODULE_EXTERNAL_LIBS
    WMDEPS::tiff
    WMDEPS::geotiff
    WMDEPS::gpkgio
    WMDEPS::gpkgext
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
    uihorizongrp.cc )

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()

