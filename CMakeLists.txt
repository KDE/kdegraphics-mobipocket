cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)

project(mobipocket)

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

include(FeatureSummary)
include(GenerateExportHeader)
include(ECMOptionalAddSubdirectory)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

set(REQUIRED_QT_VERSION 5.4.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Gui)

ecm_setup_version("2.0.0"
    VARIABLE_PREFIX QMOBIPOCKET
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/QMobipocketConfigVersion.cmake"
    SOVERSION "2"
)

add_subdirectory(lib)

ecm_optional_add_subdirectory(thumbnailers)

set(CMAKECONFIG_INSTALL_DIR "${LIB_INSTALL_DIR}/cmake/QMobipocket")

ecm_configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/QMobipocketConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/QMobipocketConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(EXPORT QMobipocketTargets
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    FILE QMobipocketTargets.cmake
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/QMobipocketConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/QMobipocketConfigVersion.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES org.kde.kdegraphics-mobipocket.metainfo.xml
        DESTINATION ${KDE_INSTALL_METAINFODIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

