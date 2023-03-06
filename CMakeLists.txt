cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0077 NEW)
cmake_minimum_required(VERSION 3.14)
project(async_json VERSION 0.3.0 LANGUAGES CXX)

include(cmake/CPM.cmake)
CPMAddPackage("gh:APokorny/hsm@0.3.1")
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.9.0")

option(async_json_PARSER_UNROLLED_SM "Build parser with unrolled state machine interpreter" OFF)
option(async_json_EXTRACTOR_UNROLLED_SM "Build secondary state machines with unrolled state machine interpreter" OFF)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  option(async_json_BUILD_TESTS "Build examples and tests" ON)
else()
  option(async_json_BUILD_TESTS "Build examples and tests" OFF)
endif()
add_library(async_json INTERFACE)
add_library(async_json::async_json ALIAS async_json)
target_link_libraries(async_json INTERFACE hsm)
target_compile_features(async_json INTERFACE cxx_std_17)
if(async_json_PARSER_UNROLLED_SM)
  target_compile_definitions(async_json INTERFACE ASYNC_JSON_PARSER_UNROLLED_SM)
endif()
if(async_json_EXTRACTOR_UNROLLED_SM)
  target_compile_definitions(async_json INTERFACE ASYNC_JSON_EXTRACTOR_UNROLLED_SM)
endif()
target_include_directories(async_json INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include>
    )
install(DIRECTORY include/async_json DESTINATION include)
packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
  DEPENDENCIES "hsm 0.2.0"
  VERSION_HEADER "${PROJECT_NAME}/version.h"
  NAMESPACE async_json
  COMPATIBILITY AnyNewerVersion
  DISABLE_VERSION_SUFFIX YES
  ARCH_INDEPENDENT YES
  )

if(async_json_BUILD_TESTS)
  CPMAddPackage("gh:catchorg/Catch2@2.13.7")
  enable_testing()

  find_program(MEMORYCHECK_COMMAND valgrind)
  set(MEMORYCHECK_COMMAND_OPTIONS --error-exitcode=1 --verbose --tool=memcheck --leak-check=full --show-leak-kinds=all
                                  --track-origins=yes --show-reachable=yes --error-limit=no --gen-suppressions=all)
  include(CTest)

  add_executable(json_parse_test test/json_parse_test.cpp)
  target_link_libraries(json_parse_test async_json Catch2::Catch2)
  add_test(NAME json_parse_test COMMAND json_parse_test)

  add_executable(json_extractor_test test/json_extractor_test.cpp)
  target_link_libraries(json_extractor_test async_json Catch2::Catch2)
  add_test(NAME json_extractor_test COMMAND json_extractor_test)

  add_executable(string_converter_test test/string_converter_test.cpp)
  target_link_libraries(string_converter_test async_json Catch2::Catch2)
  add_test(NAME string_converter_test COMMAND string_converter_test)
endif()
