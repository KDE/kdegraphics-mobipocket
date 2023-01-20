if (QT_MAJOR_VERSION STREQUAL "6")
    set(qmobipocket_INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/QMobipocket6)
else()
    set(qmobipocket_INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/QMobipocket)
endif()

add_library(qmobipocket SHARED)
set_target_properties(qmobipocket PROPERTIES
    VERSION ${QMOBIPOCKET_VERSION}
    SOVERSION ${QMOBIPOCKET_SOVERSION}
)
if (QT_MAJOR_VERSION STREQUAL "6")
    set_target_properties(qmobipocket PROPERTIES
        LIBRARY_OUTPUT_NAME QMobipocket6
        EXPORT_NAME QMobipocket6
    )
endif()

target_sources( qmobipocket PRIVATE
    decompressor.cpp
    mobipocket.cpp
    qfilestream.cpp
)

generate_export_header(qmobipocket)

target_link_libraries (qmobipocket
    PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
)

if(TARGET Qt6::Core5Compat)
    target_link_libraries(qmobipocket PRIVATE Qt6::Core5Compat)
endif()

target_include_directories(qmobipocket
    INTERFACE
        "$<INSTALL_INTERFACE:${qmobipocket_INCLUDE_INSTALL_DIR}>"
)

install(FILES
    mobipocket.h
    qfilestream.h
    ${CMAKE_CURRENT_BINARY_DIR}/qmobipocket_export.h
    DESTINATION ${qmobipocket_INCLUDE_INSTALL_DIR}/qmobipocket
    COMPONENT Devel
)

install(TARGETS qmobipocket EXPORT QMobipocketTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
