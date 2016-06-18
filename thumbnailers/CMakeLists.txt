include_directories(
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/lib
                   )

########### next target ###############

set(mobithumbnail_SRCS
  mobithumbnail.cpp
)

find_package(KF5 REQUIRED
    KIO
    KDELibs4Support
    )

add_library(mobithumbnail MODULE ${mobithumbnail_SRCS})
target_link_libraries(mobithumbnail  KF5::KDELibs4Support KF5::KIOCore Qt5::Gui qmobipocket)
install(TARGETS mobithumbnail DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES mobithumbnail.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
