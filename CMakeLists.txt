project(mobipocket)

find_package(KDE4 4.6.0 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(MacroOptionalAddSubdirectory)

if(CMAKE_SOURCE_DIR STREQUAL kdegraphics_SOURCE_DIR)
  set(MOBI_IN_KDEGRAPHICS TRUE)
endif()

if(MOBI_IN_KDEGRAPHICS)
  set(OKULAR_FOUND TRUE)
  set(OKULAR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/okular)
else()
  find_package(Okular)
  macro_log_feature(OKULAR_FOUND "okular" "Provides an Okular backend for Mobipocket documents" "http://okular.kde.org/" FALSE "" "")
endif()

find_package(Strigi)
macro_log_feature(STRIGI_FOUND "strigi libraries" "Provide a strigi analyzer for Mobipocket documents" "" FALSE "" "")

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${KDE4_INCLUDES})

macro_optional_add_subdirectory(thumbnailers)
if(OKULAR_FOUND)
  macro_optional_add_subdirectory(okular)
endif()
if(STRIGI_FOUND)
  macro_optional_add_subdirectory(strigi)
endif()

macro_display_feature_log()

