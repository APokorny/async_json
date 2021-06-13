# Async JSON

A library for parsing json in parts, avoiding the need to keep the complete document in memory. The parsing works through a statemachine with a state stack for nested arrays and objects.

## ATTENTION

In the current state \" escape sequences are supported but not comfortable replaced like in other parsers.
That means that the user has to expect escape sequencesin string views of the original content.

## TODO:
* provide utilities to optionally convert \u unicode escape symbols and similar
* optimize binary size and performance by improving hsm
* find a better way to parse floats/doubles.
* printing - or rather transformation support

## License

Boost Software License.

## Dependencies

* optional C++ std features like string_view
* hsm: https://github.com/APokorny/hsm (so transitively kvasir mpl and https://github.com/APokorny/tiny_tuple)
* Upcoming setup with fetch content will allow you to just:
```CMake
project(async_json_env LANGUAGES CXX)
cmake_minimum_required(VERSION 3.14)
cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0048 NEW)
include(FetchContent)
fetchcontent_declare(async_json 
    GIT_REPOSITORY https://github.com/APokorny/async_json
    GIT_TAG development)
fetchcontent_makeavailable(async_json)
  ...
```
* previously this was the preferred setup:

```CMake
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
```
