cmake_minimum_required(VERSION 3.10)
project(async_json LANGUAGES CXX)
cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0048 NEW)
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-std=c++17 HAS_CXX17_FLAG)
check_cxx_compiler_flag(-std=c++2a HAS_CXX20_FLAG)



if(HAS_CXX20_FLAG)
  set(CMAKE_CXX_STANDARD 20)
elseif(HAS_CXX17_FLAG)
  set(CMAKE_CXX_STANDARD 17)
else()
  message(FATAL_ERROR "requires at least c++17")
endif()


find_package(hsm)

option(async_json_BUILD_TESTS "Build examples and tests" ON)
add_library(async_json INTERFACE)
add_library(async_json::async_json ALIAS async_json)
target_link_libraries(async_json INTERFACE hsm)
target_include_directories(async_json INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include>
    )
install(TARGETS async_json EXPORT async_json-Targets DESTINATION include)
install(EXPORT async_json-Targets
    NAMESPACE async_json::
    DESTINATION lib/cmake/async_json
    )
install(DIRECTORY include/async_json DESTINATION include)

if(async_json_BUILD_TESTS)
    add_subdirectory(test)
endif()
