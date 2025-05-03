cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

project(mobipocket VERSION 3.0.0)

find_package(ECM 5.92.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(ECMAddTests)
include(ECMGenerateExportHeader)
include(ECMOptionalAddSubdirectory)
include(ECMSetupVersion)
include(ECMDeprecationSettings)
include(ECMAddTests)
include(KDEGitCommitHooks)
include(KDEClangFormat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(REQUIRED_QT_VERSION 5.12.0)
find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Gui)

if (QT_MAJOR_VERSION STREQUAL "6")
    set(CMAKECONFIG_NAME "QMobipocket6")
else()
    set(CMAKECONFIG_NAME "QMobipocket")
endif()

ecm_setup_version(PROJECT
    VARIABLE_PREFIX QMOBIPOCKET
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/lib/qmobipocket_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKECONFIG_NAME}ConfigVersion.cmake"
    SOVERSION "3"
)

ecm_set_disabled_deprecation_versions(
    QT 6.4
)

add_subdirectory(lib)
if (BUILD_TESTING)
  add_subdirectory(autotests)
endif()

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

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES lib/*.cpp lib/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

