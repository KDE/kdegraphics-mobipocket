
set (QMOBIPOCKET_SRCS
    decompressor.cpp
    mobipocket.cpp
    qfilestream.cpp
)

kde4_add_library(qmobipocket SHARED ${QMOBIPOCKET_SRCS})

target_link_libraries (qmobipocket
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)

install(FILES
    mobipocket.h
    qmobipocket_export.h
    qfilestream.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/qmobipocket/ COMPONENT Devel
)

install(TARGETS qmobipocket ${INSTALL_TARGETS_DEFAULT_ARGS})
set_target_properties(qmobipocket PROPERTIES VERSION 1.0.0 SOVERSION 1 )
