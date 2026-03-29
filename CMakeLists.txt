cmake_minimum_required(VERSION 3.16)
project(lab3_v3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

add_executable(lab3
    src/main.cpp
    src/model/package.cpp
    src/model/mainpackage.cpp
    src/model/librarypackage.cpp
    src/model/metapackage.cpp
    src/model/packagemanager.cpp
    src/model/repository.cpp
    src/model/packagefactory.cpp
    src/view/consoleui.cpp
    src/model/capabilityinterfaces.h
)

find_package(GTest REQUIRED)
enable_testing()
include(GoogleTest)

add_executable(test_lab3
    tests/main_test.cpp
    tests/unit/test_hashtable.cpp
    tests/unit/test_package.cpp
    tests/unit/test_metapackage.cpp
    tests/unit/test_librarypackage.cpp
    tests/unit/test_packagemanager.cpp
    tests/unit/test_repository.cpp
    tests/unit/test_packagefactory.cpp
    
    src/model/package.cpp
    src/model/mainpackage.cpp
    src/model/librarypackage.cpp
    src/model/metapackage.cpp
    src/model/packagemanager.cpp
    src/model/repository.cpp
    src/model/packagefactory.cpp
    src/view/consoleui.cpp
    src/model/capabilityinterfaces.h
)

target_compile_options(test_lab3 PRIVATE 
    -fprofile-arcs -ftest-coverage
)
target_link_options(test_lab3 PRIVATE 
    -fprofile-arcs -ftest-coverage -lgcov
)

target_link_libraries(test_lab3
    PRIVATE
    GTest::gtest_main
)

target_include_directories(test_lab3 PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_lab3)

add_custom_target(coverage
    COMMAND ${CMAKE_COMMAND} -E echo "Running coverage analysis..."
    COMMAND lcov --directory . --capture --output-file coverage.info
    COMMAND lcov --remove coverage.info '*/usr/*' '*/tests/*' '*/gtest/*' --output-file coverage.info
    COMMAND genhtml coverage.info --output-directory coverage_report
    COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated in coverage_report/"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generate coverage report"
    DEPENDS test_lab3
)
