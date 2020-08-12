cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

if((NOT ${EIGEN3_INCLUDE_DIR}) OR (NOT EXISTS ${EIGEN3_INCLUDE_DIR}))
    message("Unable to find Eigen")
    execute_process(COMMAND git submodule update --init -- eigen-git-mirror
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    file( COPY "eigen-git-mirror/Eigen" DESTINATION "include" )
    file( COPY "eigen-git-mirror/unsupported/Eigen" DESTINATION "include/unsupported" )
    set(EIGEN3_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

if((NOT ${NANOFLANN_INCLUDE_DIR}) OR (NOT EXISTS ${NANOFLANN_INCLUDE_DIR}))
    message("Unable to find nanoflann")
    execute_process(COMMAND git submodule update --init -- nanoflann
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/nanoflann/include/nanoflann.hpp DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include )
    file( RENAME ${CMAKE_CURRENT_SOURCE_DIR}/include/nanoflann.hpp ${CMAKE_CURRENT_SOURCE_DIR}/include/nanoflann.h ) 
    set(NANOFLANN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

if((NOT ${MBA_INCLUDE_DIR}) OR (NOT EXISTS ${MBA_INCLUDE_DIR}))
    message("Unable to find mba")
    execute_process(COMMAND git submodule update --init -- mba
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/mba/mba/mba.hpp DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include )
    set(MBA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

if((NOT ${PYBIND11_INCLUDE_DIR}) OR (NOT EXISTS ${PYBIND11_INCLUDE_DIR}))
    message("Unable to find pybind11")
    execute_process(COMMAND git submodule update --init -- pybind11
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/pybind11/include/pybind11 DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include )
    set(PYBIND11_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

