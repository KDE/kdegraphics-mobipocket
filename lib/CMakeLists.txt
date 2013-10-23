
set (QMOBIPOCKET_SRCS
    decompressor.cpp
    mobipocket.cpp
)

kde4_add_library(qmobipocket SHARED ${QMOBIPOCKET_SRCS})

target_link_libraries (qmobipocket
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)

install(FILES
    mobipocket.h
    qmobipocket_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/qmobipocket/ COMPONENT Devel
)

install(TARGETS qmobipocket ${INSTALL_TARGETS_DEFAULT_ARGS})
