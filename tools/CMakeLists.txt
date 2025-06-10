add_executable(mobidump mobidump.cpp)
target_link_libraries(mobidump
    Qt6::Core
    qmobipocket
)

# Though the dump utility is not a regular test case, running it increases
# code coverage significantly
# We check if the output matches using a regular expression

add_test(NAME dump_fail COMMAND mobidump "${CMAKE_CURRENT_SOURCE_DIR}")
set_tests_properties(dump_fail PROPERTIES
    # Directories are not valid arguments
    WILL_FAIL TRUE
)

add_test(NAME dump_too_many_args COMMAND mobidump "${CMAKE_CURRENT_SOURCE_DIR}" "xxx")
set_tests_properties(dump_too_many_args PROPERTIES
    WILL_FAIL TRUE
)

add_test(NAME dump_file_not_found COMMAND mobidump "${CMAKE_CURRENT_SOURCE_DIR}/does_not_exist")
set_tests_properties(dump_file_not_found PROPERTIES
    # Directories are not valid arguments
    WILL_FAIL TRUE
)

add_test(NAME dump_meta COMMAND mobidump "${CMAKE_CURRENT_SOURCE_DIR}/../autotests/testdata/test.mobi")
set_tests_properties(dump_meta PROPERTIES
    PASS_REGULAR_EXPRESSION "The Big Brown Bear"
)

add_test(NAME dump_text COMMAND mobidump "-f" "${CMAKE_CURRENT_SOURCE_DIR}/../autotests/testdata/test.mobi")
set_tests_properties(dump_text PROPERTIES
    PASS_REGULAR_EXPRESSION "This is a sample"
)
