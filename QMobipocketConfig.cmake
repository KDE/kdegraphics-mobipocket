get_filename_component( _currentDir  ${CMAKE_CURRENT_LIST_FILE} PATH)
get_filename_component( _currentDir  ${_currentDir} PATH)
get_filename_component( _currentDir  ${_currentDir} PATH)
get_filename_component( _currentDir  ${_currentDir} PATH)

# find the full paths to the library and the includes:
find_path(QMOBIPOCKET_INCLUDE_DIR qmobipocket/mobipocket.h
          HINTS ${_currentDir}/include
          NO_DEFAULT_PATH)

find_library(QMOBIPOCKET_LIBRARY qmobipocket 
             HINTS ${_currentDir}/lib
             NO_DEFAULT_PATH)

set(QMOBIPOCKET_LIBRARIES ${QMOBIPOCKET_LIBRARY})

if(QMOBIPOCKET_INCLUDE_DIR AND QMOBIPOCKET_LIBRARY)
    set(QMOBIPOCKET_FOUND TRUE)
endif()

