find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

add_executable(bitreadertest_bin bitreadertest.cpp)
target_link_libraries(bitreadertest_bin
    Qt6::Test
)
ecm_mark_as_test(bitreadertest_bin)

# Run the benchmarks with just 1 iteration during CI, so we known it works
add_test(NAME bitreadertest COMMAND bitreadertest_bin "-iterations" "1")


add_executable(decompressortest_bin
    decompressortest.cpp
    ../lib/decompressor.cpp
)
target_link_libraries(decompressortest_bin
    Qt6::Test
)
ecm_mark_as_test(decompressortest_bin)

# Run the benchmarks with just 1 iteration during CI, so we known it works
add_test(NAME decompressortest COMMAND decompressortest_bin "-iterations" "1")

configure_file(testsconfig.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/testsconfig.h @ONLY)

ecm_add_test(mobipockettest.cpp
    TEST_NAME "mobipockettest"
    LINK_LIBRARIES
        Qt6::Test
        qmobipocket
)
