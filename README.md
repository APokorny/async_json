# Async JSON

A library for parsing json in parts avoiding the need to keep the complete document in memory. The parsing works through a statemachine with a state stack for nested arrays and objects.

## License

Boost Software License.

## Dependencies

* optional C++ std features like string_view
* hsm: github.com/APokorny/hsm (so transitively kvasir mpl and github.com/APokorny/tiny_tuple)

* Example setup: 
'''
project(async_json_env LANGUAGES CXX)
cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0048 NEW)
set(as_subproject
    kvasir_mpl hsm tiny_tuple)

macro(find_package)
    if(NOT ${ARGV0} IN_LIST as_subproject)
        _find_package(${ARGV})
    endif()
endmacro()

add_subdirectory(mpl)
add_subdirectory(tiny-tuple)
add_subdirectory(hsm)
add_subdirectory(async_json)
'''
