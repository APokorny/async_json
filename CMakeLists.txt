cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0077 NEW)
cmake_minimum_required(VERSION 3.14)
project(async_json VERSION 0.2.0 LANGUAGES CXX)

include(cmake/CPM.cmake)
CPMAddPackage(NAME hsm
  VERSION 0.2.0
  GITHUB_REPOSITORY "APokorny/hsm"
  GIT_TAG development)
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.6.0")

option(async_json_BUILD_TESTS "Build examples and tests" ON)
add_library(async_json INTERFACE)
add_library(async_json::async_json ALIAS async_json)
target_link_libraries(async_json INTERFACE hsm)
target_compile_features(async_json INTERFACE cxx_std_17)
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
  DEPENDENCIES "tiny_tuple 0.1.0"
  VERSION_HEADER "${PROJECT_NAME}/version.h"
  EXPORT_HEADER "${PROJECT_NAME}/export.h"
  NAMESPACE hsm
  COMPATIBILITY AnyNewerVersion
  DISABLE_VERSION_SUFFIX YES
  ARCH_INDEPENDENT YES
  )

if(async_json_BUILD_TESTS)
    add_subdirectory(test)
endif()
