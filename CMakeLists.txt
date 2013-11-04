project(mobipocket)

find_package(KDE4 4.6.0 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(MacroOptionalAddSubdirectory)

find_package(Strigi)
macro_log_feature(STRIGI_FOUND "strigi libraries" "Provide a strigi analyzer for Mobipocket documents" "" FALSE "" "")

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${KDE4_INCLUDES})

add_subdirectory(lib)

macro_optional_add_subdirectory(thumbnailers)
if(STRIGI_FOUND)
  macro_optional_add_subdirectory(strigi)
endif()

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/QMobipocketConfig.cmake
        DESTINATION ${LIB_INSTALL_DIR}/cmake/QMobipocket )

macro_display_feature_log()

