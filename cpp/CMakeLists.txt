cmake_minimum_required(VERSION 3.28)
project(AnomalyDetector VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(PROJECT_ROOT ${CMAKE_SOURCE_DIR}/..)

set(ABSL_ENABLE_INSTALL ON CACHE BOOL "" FORCE)
set(RE2_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(${PROJECT_ROOT}/external/abseil-cpp ${CMAKE_BINARY_DIR}/external/abseil-cpp)
add_subdirectory(${PROJECT_ROOT}/external/re2 ${CMAKE_BINARY_DIR}/external/re2)

add_library(utils_lib utils/utils.cpp)
add_library(io_manager_lib io_manager/reader.cpp)
add_library(parser_lib parser/parser.cpp)
add_library(detector_lib detector/detector.cpp)
add_library(reporter_lib reporter/reporter.cpp)
add_library(socket_server_lib socket_server/socket_server.cpp)

target_include_directories(utils_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(io_manager_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(parser_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(detector_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(reporter_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(socket_server_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(parser_lib PRIVATE re2)

target_link_libraries(reporter_lib PRIVATE detector_lib parser_lib utils_lib re2)

add_executable(anomaly_detector main.cpp)
target_link_libraries(anomaly_detector PRIVATE io_manager_lib parser_lib detector_lib reporter_lib socket_server_lib re2)

add_custom_target(run COMMAND $<TARGET_FILE:anomaly_detector> ${PROJECT_ROOT}/logs/server.log DEPENDS anomaly_detector WORKING_DIRECTORY ${PROJECT_ROOT} COMMENT "Running anomaly detector on server.log\n")