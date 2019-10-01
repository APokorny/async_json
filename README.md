# Async JSON

A library for parsing json in parts avoiding the need to keep the complete document in memory. The parsing works through a statemachine with a state stack for nested arrays and objects.

## License

Boost Software License.

## Dependencies

* optional C++ std features like string_view
* hsm: github.com/APokorny/hsm
