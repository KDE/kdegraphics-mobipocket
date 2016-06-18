
set (QMOBIPOCKET_SRCS
    decompressor.cpp
    mobipocket.cpp
    qfilestream.cpp
)

add_library(qmobipocket SHARED ${QMOBIPOCKET_SRCS})

target_link_libraries (qmobipocket
    Qt5::Core
    Qt5::Gui
)

install(FILES
    mobipocket.h
    qmobipocket_export.h
    qfilestream.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/qmobipocket/ COMPONENT Devel
)

install(TARGETS qmobipocket ${INSTALL_TARGETS_DEFAULT_ARGS})
set_target_properties(qmobipocket PROPERTIES VERSION 1.0.0 SOVERSION 1 )
