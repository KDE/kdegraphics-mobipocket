add_library(qmobipocket SHARED)
set_target_properties(qmobipocket PROPERTIES
    VERSION ${QMOBIPOCKET_VERSION}
    SOVERSION ${QMOBIPOCKET_SOVERSION}
)

target_sources( qmobipocket PRIVATE
    decompressor.cpp
    mobipocket.cpp
    qfilestream.cpp
)

generate_export_header(qmobipocket)

target_link_libraries (qmobipocket
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
)

if(TARGET Qt6::Core5Compat)
    target_link_libraries(qmobipocket Qt6::Core5Compat)
endif()

install(FILES
    mobipocket.h
    qfilestream.h
    ${CMAKE_CURRENT_BINARY_DIR}/qmobipocket_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/qmobipocket/ COMPONENT Devel
)

install(TARGETS qmobipocket EXPORT QMobipocketTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
