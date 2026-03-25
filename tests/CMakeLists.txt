cmake_minimum_required(VERSION 3.16)

# GTest from Homebrew (arm64 native)
find_package(GTest REQUIRED
    HINTS /opt/homebrew/lib/cmake/GTest
          /opt/homebrew/Cellar/googletest/1.17.0/lib/cmake/GTest
)

# Coverage flags
function(add_cp_test TEST_NAME SOURCE_FILE)
    add_executable(${TEST_NAME} ${SOURCE_FILE})
    target_link_libraries(${TEST_NAME}
        PRIVATE
            GTest::gtest_main
            rte bsw_ecum bsw_dem swc_engine swc_battery
    )
    if(ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${TEST_NAME} PRIVATE --coverage -fprofile-arcs -ftest-coverage)
        target_link_options(${TEST_NAME} PRIVATE --coverage)
    endif()
    include(GoogleTest)
    gtest_discover_tests(${TEST_NAME})
endfunction()

add_cp_test(test_rte         unit/test_rte.cpp)
add_cp_test(test_swc_engine  unit/test_swc_engine.cpp)
add_cp_test(test_swc_battery unit/test_swc_battery.cpp)
