SET(OD_IS_PLUGIN yes)
set ( OD_MODULE_DEPS Basic )
#SET(OD_TEST_PROGS efd_testing.cc eigentools_testing.cc)

SET(OD_MODULE_SOURCES
    eigentoolspi.cc
    efd.cc
)

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
