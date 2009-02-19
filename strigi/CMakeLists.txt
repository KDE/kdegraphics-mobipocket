find_package(Strigi REQUIRED)

include_directories(${KDE4_INCLUDES} ${QT_INCLUDES}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${STRIGI_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/libs/mobipocket
                   )

########### next target ###############

find_package(SharedMimeInfo REQUIRED)

set(mobianalyzer_SRCS
  ${PROJECT_SOURCE_DIR}/libs/mobipocket/mobipocket.cpp
  ${PROJECT_SOURCE_DIR}/libs/mobipocket/decompressor.cpp
  mobiendanalyzer.cpp
)

kde4_add_library(mobi MODULE ${mobianalyzer_SRCS})

target_link_libraries(mobi ${STRIGI_STREAMS_LIBRARY} ${STRIGI_STREAMANALYZER_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

# install the module in the right directory so it is picked up
set_target_properties(mobi PROPERTIES PREFIX ${STRIGI_LINE_ANALYZER_PREFIX})

########### install files ###############

install(TARGETS mobi LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)
