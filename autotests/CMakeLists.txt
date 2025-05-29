# SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
# SPDX-Licence-Identifier: BSD-2-Clauses

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

add_executable(bitreadertest_bin bitreadertest.cpp)
target_link_libraries(bitreadertest_bin
    Qt${QT_MAJOR_VERSION}::Test
)
ecm_mark_as_test(bitreadertest_bin)

# Run the benchmarks with just 1 iteration during CI, so we known it works
add_test(NAME bitreadertest COMMAND bitreadertest_bin "-iterations" "1")

add_executable(texttest_bin texttest.cpp)
target_link_libraries(texttest_bin
    Qt${QT_MAJOR_VERSION}::Test
    qmobipocket
)
ecm_mark_as_test(texttest_bin)

# Run the benchmarks with just 1 iteration during CI, so we known it works
add_test(NAME texttest COMMAND texttest_bin "-iterations" "1")

add_definitions(-DDATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/testdata")

ecm_add_tests(kpdbtest.cpp metadatatest.cpp
    LINK_LIBRARIES Qt::Test qmobipocket
)
