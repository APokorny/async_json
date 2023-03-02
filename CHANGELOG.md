
# Changelog

## Version 0.2.0 - 2nd March 2023

**Feature**: build system
- move towards fetch content and simpler builds
- move towards CPM to make it even nicer
- set fixed revision for all dependencies
- bump to 0.2.0 for hsm for greater performance across the board

**Feature**: Extern template for json parser
- use ASYNC_JSON_EXTERN to split out implementations in separate translation units
- include exstern_impl when you are happy with the default traits, otherwise create 
  a similar file with you preferred value types

**Feature**: Improved json extractor
- allow extracted values into std::containers
- simple helper on_exit on_enter

**Feature**: Dropping the handler interface
- parser now directly creates events passed to a single function

**Feature**: Usage of std::variant 

**Feature**: clang warnings fixed

## Version 0.1.0

Initial release with basic json parsing capabilities and parse event filters.
