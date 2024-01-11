
# Changelog

## Version 0.3.2 11th of Junary 2024 

**Bugfix**:
- fix escape sequence replacement
- minor improvements to fix clangs new -Wunsafe-buffer warnings

## Version 0.3.1 27th of March 2023

**Feature**: Bump to HSM 0.3.5
- FTBFS fix in unrolled state machine
- warnings fixed

## Version 0.3.0 9th of March 2023

**Feature**: Bump to HSM 0.3.3 
- This allows async_json to benefit from the new hsm backend

**Feature**: Fast parser mode
- Based on the new backend a fast parser mode is added - it can be used to trade ROM for performance:
- Early tests shows that the new mode is twice as fast with gcc-12
- The space efficient mode is still the default
- The feature is enabled in basic_path, basic_json_parser and on_array_element via a tag type: `unrolled_tag` vs `table_tag`
- Use the 'fast' variants of extractor, path, on_array_element.

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
