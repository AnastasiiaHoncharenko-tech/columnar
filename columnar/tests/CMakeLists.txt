add_executable(columnar_tests
        test_main.cpp
        test_csv_reading.cpp
        test_column_access.cpp
        test_filtering.cpp
)

target_link_libraries(columnar_tests
        PRIVATE
        columnar
        GTest::gtest
        GTest::gtest_main
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include(GoogleTest)
gtest_add_tests(TARGET columnar_tests)