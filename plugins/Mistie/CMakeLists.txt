SET(OD_MODULE_DEPS AttributeEngine)
SET(OD_IS_PLUGIN yes)
SET(OD_MODULE_SOURCES
    mistiepi.cc
    mistieapplier.cc
    mistiecordata.cc
    mistiedata.cc
    mistieestimator.cc
    trcanalysis.cc
    mistieestimator2d3d.cc
)

find_package(Eigen3)
list( APPEND OD_MODULE_INCLUDESYSPATH ${EIGEN3_INCLUDE_DIR} )
add_definitions( -DNDEBUG )

SET( OD_PLUGIN_ALO_EXEC ${OD_ATTRIB_EXECS} ${OD_VOLUME_EXECS} )
OD_INIT_MODULE()
