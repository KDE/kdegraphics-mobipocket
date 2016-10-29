
set (QMOBIPOCKET_SRCS
    decompressor.cpp
    mobipocket.cpp
    qfilestream.cpp
)

add_library(qmobipocket SHARED ${QMOBIPOCKET_SRCS})
generate_export_header(qmobipocket)

target_link_libraries (qmobipocket
    Qt5::Core
    Qt5::Gui
)

install(FILES
    mobipocket.h
    qfilestream.h
    ${CMAKE_CURRENT_BINARY_DIR}/qmobipocket_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/qmobipocket/ COMPONENT Devel
)

install(TARGETS qmobipocket EXPORT QMobipocketTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
set_target_properties(qmobipocket PROPERTIES VERSION ${QMOBIPOCKET_VERSION} SOVERSION ${QMOBIPOCKET_SOVERSION})
