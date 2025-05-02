find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

add_executable(bitreadertest_bin bitreadertest.cpp)
target_link_libraries(bitreadertest_bin
    Qt${QT_MAJOR_VERSION}::Test
)
ecm_mark_as_test(bitreadertest_bin)

# Run the benchmarks with just 1 iteration during CI, so we known it works
add_test(NAME bitreadertest COMMAND bitreadertest_bin "-iterations" "1")


add_executable(decompressortest_bin
    decompressortest.cpp
    ../lib/decompressor.cpp
)
target_link_libraries(decompressortest_bin
    Qt${QT_MAJOR_VERSION}::Test
)
ecm_mark_as_test(decompressortest_bin)

# Run the benchmarks with just 1 iteration during CI, so we known it works
add_test(NAME decompressortest COMMAND decompressortest_bin "-iterations" "1")

add_definitions(-DDATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/testdata")

ecm_add_tests(kpdbtest.cpp
    LINK_LIBRARIES Qt::Test qmobipocket
)
