function(make_includable input_file output_file)
    file(READ ${input_file} content)
    set(delim "===")
    set(content "static const char* mistie_report = R\"${delim}(\n${content})${delim}\";")
    file(WRITE ${output_file} "${content}")
endfunction(make_includable)

if(${CMAKE_CURRENT_SOURCE_DIR}/mistie_report_template.in IS_NEWER_THAN ${CMAKE_CURRENT_SOURCE_DIR}/mistie_report.h)
    make_includable(${CMAKE_CURRENT_SOURCE_DIR}/mistie_report_template.in ${CMAKE_CURRENT_SOURCE_DIR}/mistie_report.h)
endif()

set(OD_MODULE_NAME uiMistie)
set(OD_IS_PLUGIN yes )
set(OD_MODULE_DEPS uiODMain uiWGMHelp  Mistie)
set(OD_MODULE_SOURCES
    uimistieestimatemainwin.cc
    uimistieanalysismainwin.cc
    uimistiecorrmainwin.cc
    uicorrviewer.cc
	uimistieapplier.cc
	uimistiepi.cc
	uimistiecorrhordlg.cc)

add_definitions( -DNDEBUG )

SET( OD_PLUGIN_ALO_EXEC ${OD_MAIN_EXEC} )
OD_INIT_MODULE()
