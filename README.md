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

* In theory only c++17, but the dependencies of async_json already require c++20
* hsm: https://github.com/APokorny/hsm (so transitively kvasir mpl and https://github.com/APokorny/tiny_tuple)
* Everything is being setup with CPM:
```CMake
project(your_project LANGUAGES CXX)
cmake_minimum_required(VERSION 3.14)
cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0048 NEW)
include(cmake/CPM.cmake)
cpmaddpacakge("gh:APokorny/async_json@0.3.0") 
  ...
```

