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
)

if (QT_MAJOR_VERSION STREQUAL "5")
    set(_generate_export_header_version_args)
else()
    # For Qt6/KF6 world transitively include the version header
    if(ECM_VERSION VERSION_LESS "5.106")
        set(include_version_header_code "#include <qmobipocket_version.h>\n")
        set(_generate_export_header_version_args CUSTOM_CONTENT_FROM_VARIABLE include_version_header_code)
    else()
        set(_generate_export_header_version_args USE_VERSION_HEADER)
    endif()
endif()

ecm_generate_export_header(qmobipocket
     VERSION ${QMOBIPOCKET_VERSION}
     DEPRECATED_BASE_VERSION 0
    ${_generate_export_header_version_args}
)

target_link_libraries (qmobipocket
    PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
)

target_include_directories(qmobipocket
    INTERFACE
        "$<INSTALL_INTERFACE:${qmobipocket_INCLUDE_INSTALL_DIR}>"
)

install(FILES
    mobipocket.h
    ${CMAKE_CURRENT_BINARY_DIR}/qmobipocket_export.h
    DESTINATION ${qmobipocket_INCLUDE_INSTALL_DIR}/qmobipocket
    COMPONENT Devel
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/qmobipocket_version.h
    DESTINATION ${qmobipocket_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(TARGETS qmobipocket EXPORT QMobipocketTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
