cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(ABSL_ENABLE_INSTALL ON CACHE BOOL "" FORCE)
set(RE2_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(${PROJECT_ROOT}/external/abseil-cpp ${CMAKE_BINARY_DIR}/external/abseil-cpp)
add_subdirectory(${PROJECT_ROOT}/external/re2 ${CMAKE_BINARY_DIR}/external/re2)

include(FetchContent)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.4
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(benchmark)

add_library(utils_lib ${PROJECT_ROOT}/cpp/utils/utils.cpp)
add_library(parser_lib ${PROJECT_ROOT}/cpp/parser/parser.cpp)
add_library(reader_lib ${PROJECT_ROOT}/cpp/io_manager/reader.cpp)
add_library(detector_lib ${PROJECT_ROOT}/cpp/detector/detector.cpp)
add_library(reporter_lib ${PROJECT_ROOT}/cpp/reporter/reporter.cpp)

target_link_libraries(reader_lib PRIVATE utils_lib)
target_link_libraries(parser_lib PUBLIC re2)
target_link_libraries(detector_lib PRIVATE utils_lib)
target_link_libraries(reporter_lib PRIVATE detector_lib utils_lib)

target_include_directories(reader_lib PRIVATE ${PROJECT_ROOT}/cpp)
target_include_directories(detector_lib PRIVATE ${PROJECT_ROOT}/cpp)

add_library(reader_bm reader_benchmark.cpp)
add_library(parser_bm parser_benchmark.cpp)
add_library(detector_bm detector_benchmark.cpp)
add_library(reporter_bm reporter_benchmark.cpp)

target_link_libraries(reader_bm PUBLIC reader_lib)
target_link_libraries(parser_bm PUBLIC parser_lib)
target_link_libraries(detector_bm PUBLIC parser_lib detector_lib)
target_link_libraries(reporter_bm PUBLIC reporter_lib parser_lib)

target_include_directories(reader_bm PRIVATE ${PROJECT_ROOT}/cpp)
target_include_directories(detector_bm PRIVATE ${PROJECT_ROOT}/cpp)

add_executable(benchmark_test main.cpp reader_benchmark.cpp parser_benchmark.cpp detector_benchmark.cpp reporter_benchmark.cpp)

target_link_libraries(benchmark_test PRIVATE benchmark::benchmark parser_bm detector_bm reader_bm reporter_bm)
target_include_directories(benchmark_test PRIVATE ${PROJECT_ROOT}/cpp)