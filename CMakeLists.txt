cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)

project(mobipocket)

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(GenerateExportHeader)
include(ECMOptionalAddSubdirectory)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt5 5.4 CONFIG REQUIRED Core Gui)

add_subdirectory(lib)

ecm_optional_add_subdirectory(thumbnailers)

find_package(Strigi)
set_package_properties(Strigi PROPERTIES DESCRIPTION "Provide a strigi analyser for Mobipocket documents"
    URL "http://strigi.sourceforge.net"
    TYPE OPTIONAL
)
if(STRIGI_STREAMS_LIBRARY)
    ecm_optional_add_subdirectory(strigi)
endif()

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/QMobipocketConfig.cmake
        DESTINATION ${LIB_INSTALL_DIR}/cmake/QMobipocket )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

