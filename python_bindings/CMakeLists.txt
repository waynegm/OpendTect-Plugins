#
# OpendTect stuff
set ( OpendTect_DIR "" CACHE PATH "OpendTect directory" )
list(APPEND CMAKE_MODULE_PATH "${OpendTect_DIR}/CMakeModules")
include( ${OpendTect_DIR}/CMakeModules/ODPlatformUtils.cmake )
set ( OpendTect_Include_DIR ${OpendTect_DIR}/include )
set ( OD_DEPS EarthModel Well Algo CRS General Geometry Basic )
foreach ( dep ${OD_DEPS} )
  include_directories( SYSTEM ${OpendTect_Include_DIR}/${dep} )
endforeach()

set ( OD_RUNTIMELIBS ${OD_DEPS})
set ( OD_LIB_OUTPUT_RELPATH bin/${OD_PLFSUBDIR}/${CMAKE_BUILD_TYPE} )
set ( OD_RUNTIMELIBPATH ${OpendTect_DIR}/${OD_LIB_OUTPUT_RELPATH} )
message("${OD_RUNTIMELIBPATH}")
if (UNIX)
    SET(CMAKE_INSTALL_RPATH "\$ORIGIN/")
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()
if (NOT PYTHON_EXECUTABLE)
    if (UNIX)
	set(PYTHON_EXECUTABLE ${OpendTect_DIR}/../Python/bin/python)
    else()
	set(PYTHON_EXECUTABLE ${OpendTect_DIR}/../Python/pythonw)
    endif()
endif()


#
#
set(PYBIND11_CPP_STANDARD -std=c++14)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/pybind11)

set ( PYMODULES
	wmodpy
)
link_directories( ${OD_RUNTIMELIBPATH} )
foreach ( module ${PYMODULES} )
  file (GLOB MOD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/wmodpy/*.cc")
  pybind11_add_module(${module} SYSTEM ${MOD_FILES})
  target_link_libraries( ${module} PRIVATE ${OD_RUNTIMELIBS} )
  if (UNIX)
    install( TARGETS ${module} COMPONENT python LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/${OD_PLFSUBDIR}/ )
  else()
    install( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Release DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/${OD_PLFSUBDIR} )
  endif()
endforeach()

