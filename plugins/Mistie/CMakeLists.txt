SET(OD_MODULE_DEPS AttributeEngine EarthModel)
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

add_definitions( -DNDEBUG )

SET( OD_PLUGIN_ALO_EXEC ${OD_ATTRIB_EXECS} ${OD_VOLUME_EXECS} )
OD_INIT_MODULE()
