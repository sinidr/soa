
cmake_minimum_required(VERSION 3.30)
project(soa LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(test_settings INTERFACE)

target_include_directories(test_settings INTERFACE 
    include
)

target_compile_features(test_settings INTERFACE 
    cxx_std_20
)

target_compile_options(test_settings INTERFACE 
    -Wall 
    -Wpedantic 
    -Werror
    -Wnrvo
)


include(cmake/TestCoverage.cmake)
soa_enable_coverage(test_settings)

include(cmake/CPM.cmake)
cpmaddpackage("gh:fmtlib/fmt#11.1.4")
cpmaddpackage("gh:catchorg/Catch2@3.8.1")
cpmaddpackage("gh:lefticus/tools#update_build_system")
target_link_libraries(test_settings INTERFACE
  fmt::fmt
  Catch2::Catch2WithMain
  lefticus::tools
)

include(CTest)

add_executable(MultiArrayList_test test/MultiArrayList.cpp)
target_link_libraries(MultiArrayList_test PRIVATE test_settings)
add_test(MultiArrayList_test MultiArrayList_test)
