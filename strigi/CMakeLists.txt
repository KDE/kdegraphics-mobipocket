include_directories(
                    ${STRIGI_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/lib
                   )

########### next target ###############

set(mobianalyzer_SRCS
  mobiendanalyzer.cpp
)

kde4_add_library(mobi MODULE ${mobianalyzer_SRCS})

target_link_libraries(mobi ${STRIGI_STREAMS_LIBRARY} ${STRIGI_STREAMANALYZER_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} qmobipocket)

# install the module in the right directory so it is picked up
set_target_properties(mobi PROPERTIES PREFIX ${STRIGI_LINE_ANALYZER_PREFIX})

########### install files ###############

install(TARGETS mobi LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)
