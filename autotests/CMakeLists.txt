find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

add_executable(bitreadertest_bin bitreadertest.cpp)
target_link_libraries(bitreadertest_bin
    Qt${QT_MAJOR_VERSION}::Test
)
ecm_mark_as_test(bitreadertest_bin)

# Run the benchmarks with just 1 iteration during CI, so we known it works
add_test(NAME bitreadertest COMMAND bitreadertest_bin "-iterations" "1")
