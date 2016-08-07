include_directories(${STRIGI_INCLUDE_DIR})

########### next target ###############

set(mobianalyzer_SRCS
  mobiendanalyzer.cpp
)

add_library(mobi MODULE ${mobianalyzer_SRCS})

target_link_libraries(mobi ${STRIGI_STREAMS_LIBRARY} ${STRIGI_STREAMANALYZER_LIBRARY}
    Qt5::Core Qt5::Gui qmobipocket)

# install the module in the right directory so it is picked up
set_target_properties(mobi PROPERTIES PREFIX strigila_)

########### install files ###############

install(TARGETS mobi LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)
