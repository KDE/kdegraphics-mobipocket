set(qmobipocket_INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/QMobipocket6)

ecm_qt_declare_logging_category(debug_SRCS
    HEADER qmobipocket_debug.h
    IDENTIFIER QMOBIPOCKET_LOG
    CATEGORY_NAME org.kde.mobipocket
    DESCRIPTION "Mobipocket"
    EXPORT QMOBIPOCKET
)

add_library(qmobipocket SHARED)
set_target_properties(qmobipocket PROPERTIES
    VERSION ${QMOBIPOCKET_VERSION}
    SOVERSION ${QMOBIPOCKET_SOVERSION}
    LIBRARY_OUTPUT_NAME QMobipocket6
    EXPORT_NAME QMobipocket6
    )

target_sources( qmobipocket PRIVATE
    decompressor.cpp
    mobipocket.cpp
    pdb.cpp
    ${debug_SRCS}
)

ecm_generate_export_header(qmobipocket
     VERSION ${QMOBIPOCKET_VERSION}
     DEPRECATED_BASE_VERSION 0
     USE_VERSION_HEADER}
)

target_link_libraries (qmobipocket
    PUBLIC
    Qt6::Core
    Qt6::Gui
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

ecm_qt_install_logging_categories(
    EXPORT QMOBIPOCKET
    FILE qmobipocket.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
