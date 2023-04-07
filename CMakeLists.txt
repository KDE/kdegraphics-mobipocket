cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

project(mobipocket VERSION 2.1.0)

find_package(ECM 5.92.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(FeatureSummary)
include(ECMGenerateExportHeader)
include(ECMOptionalAddSubdirectory)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMDeprecationSettings)

set(REQUIRED_QT_VERSION 5.12.0)
find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Gui)

if(TARGET Qt6::Core)
    find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
endif()

if (QT_MAJOR_VERSION STREQUAL "6")
    set(CMAKECONFIG_NAME "QMobipocket6")
else()
    set(CMAKECONFIG_NAME "QMobipocket")
endif()

ecm_setup_version(PROJECT
    VARIABLE_PREFIX QMOBIPOCKET
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/lib/qmobipocket_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKECONFIG_NAME}ConfigVersion.cmake"
    SOVERSION "2"
)

ecm_set_disabled_deprecation_versions(
    QT 6.4
)

add_subdirectory(lib)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/${CMAKECONFIG_NAME}")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/QMobipocketConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKECONFIG_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(EXPORT QMobipocketTargets
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    FILE ${CMAKECONFIG_NAME}Targets.cmake
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKECONFIG_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKECONFIG_NAME}ConfigVersion.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

