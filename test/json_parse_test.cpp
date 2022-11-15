#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <iostream>
#include <iomanip>
#include <vector>
#include <async_json/basic_json_parser.hpp>
#include <async_json/saj_event_value.hpp>
#include <catch2/catch.hpp>

namespace a = async_json;

constexpr const char* error_str(a::error_cause cause);

struct call
{
    a::saj_event callt;
    call() = default;
    call(a::saj_event ev, ptrdiff_t d = 0, char const* str = "", double f = 0.0) : callt{ev}, value{d}, buf{str}, float_val{f} {}
    template <typename Traits>
    call(a::saj_event_value<Traits> const& ev) : callt{ev.event}
    {
        using a::saj_event;
        if (callt == saj_event::string_value_start || callt == saj_event::object_name_start)
            buf = ev.as_string_view();
        else if (callt == saj_event::integer_value)
            value = ev.as_number();
        else if (callt == saj_event::boolean_value)
            value = ev.as_bool();
        else if (callt == saj_event::parse_error)
            value = ev.as_error_cause();
        else if (callt == saj_event::float_value)
            float_val = ev.as_float_number();
    }
    ptrdiff_t   value{0};
    std::string buf;
    double      float_val{0.0};

    friend std::ostream& operator<<(std::ostream& o, call const& rhs)
    {
        switch (rhs.callt)
        {
            case a::saj_event::parse_error: return o << "invalid json: " << error_str(static_cast<a::error_cause>(rhs.value));
            case a::saj_event::boolean_value: return o << std::boolalpha << bool(rhs.value);
            case a::saj_event::null_value: return o << "null";
            case a::saj_event::object_name_start: return o << '"' << rhs.buf << '"' << ":";
            case a::saj_event::object_start: return o << "{";
            case a::saj_event::object_end: return o << "}";
            case a::saj_event::array_start: return o << "[";
            case a::saj_event::array_end: return o << "]";
            case a::saj_event::string_value_start: return o << '"' << rhs.buf << '"' << ":";
            case a::saj_event::integer_value: return o << rhs.value;
            case a::saj_event::float_value: return o << rhs.float_val;
        }
        return o;
    }
    constexpr bool operator==(call const& rhs) const
    {
        return callt == rhs.callt && value == rhs.value && buf == rhs.buf && float_val == rhs.float_val;
    }
    constexpr bool operator!=(call const& rhs) const { return !(*this == rhs); }
};
constexpr const char* error_str(a::error_cause cause)
{
    switch (cause)
    {
        case a::no_error: return "no error";
        case a::wrong_keyword_character: return "no error";
        case a::mismatched_array: return "mismatched array braces";
        case a::mismatched_brace: return "mismatched parenthesis";
        case a::member_exp: return "member expected";
        case a::colon_exp: return "colon expected";
        case a::unexpected_character: return "unexpected character";
        case a::comma_expected: return "comma expected";
        case a::invalid_number: return "invalid character in number";
        default: return "no error";
    }
}

struct sp_float
{
    using float_t   = float;
    using integer_t = int;
    using sv_t      = async_json::default_traits::sv_t;
};

template <typename T = async_json::default_traits>
struct test_handler
{
    std::vector<call> calls;
    using traits    = T;
    using float_t   = typename traits::float_t;
    using integer_t = typename traits::integer_t;
    using sv_t      = typename traits::sv_t;

    void operator()(a::saj_event_value<T> const& value)
    {
        // filters out _end events and appends all _cont tokens to start
        if (value.event == a::saj_event::string_value_cont || value.event == a::saj_event::object_name_cont)
            calls.back().buf += std::string(value.as_string_view());
        else if (value.event != a::saj_event::string_value_end && value.event != a::saj_event::object_name_end)
            calls.push_back(call(value));
    }
    friend std::ostream& operator<<(std::ostream& o, test_handler const& rhs)
    {
        for (auto const& element : rhs.calls) o << element;
        return o;
    }
};

TEST_CASE("Detect keywords: null")
{
    char const                           input_buffer[] = "   null   ";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::null_value, 0}}));
}

TEST_CASE("Detect keywords: true")
{
    char const                           input_buffer[] = "   true ";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::boolean_value, 1}}));
}

TEST_CASE("Detect keywords: false")
{
    char const                           input_buffer[] = " false ";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::boolean_value, 0}}));
}

TEST_CASE("Error in keyword: false")
{
    char const                           input_buffer[] = " fase ";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls,
                 Catch::Matchers::Equals(std::vector<call>{{a::saj_event::parse_error, a::wrong_keyword_character}}));
}
TEST_CASE("parse empty object")
{
    char const                           input_buffer[] = R"( {} )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls,
                 Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start}, {a::saj_event::object_end}}));
}
TEST_CASE("Error: object expects member name")
{
    char const                           input_buffer[] = R"( { [ }] )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls,
                 Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start}, {a::saj_event::parse_error, a::member_exp}}));
}

TEST_CASE("Parse named object")
{
    char const                           input_buffer[] = R"( { "blubber": {} } )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                                                        {a::saj_event::object_name_start, 0, "blubber"},
                                                                                        {a::saj_event::object_start},
                                                                                        {a::saj_event::object_end},
                                                                                        {a::saj_event::object_end}}));
}

TEST_CASE("Parse empty array")
{
    char const                           input_buffer[] = R"( { "blubber": [] } )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                                                        {a::saj_event::object_name_start, 0, "blubber"},
                                                                                        {a::saj_event::array_start},
                                                                                        {a::saj_event::array_end},
                                                                                        {a::saj_event::object_end}}));
}

TEST_CASE("Error unmatched array close")
{
    char const                           input_buffer[] = R"( { "blubber": ] } )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(
        p.callback_handler()->calls,
        Catch::Matchers::Equals(std::vector<call>{
            {a::saj_event::object_start}, {a::saj_event::object_name_start, 0, "blubber"}, {a::saj_event::parse_error, a::unexpected_character}}));
}

TEST_CASE("Error object members must be named")
{
    char const                           input_buffer[] = R"( { "blubber": {}, {} } )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                                                        {a::saj_event::object_name_start, 0, "blubber"},
                                                                                        {a::saj_event::object_start},
                                                                                        {a::saj_event::object_end},
                                                                                        {a::saj_event::parse_error, a::member_exp}}));
}

TEST_CASE("String values")
{
    char const                           input_buffer[] = R"( { "mmm1": "wert", "mem2": "wert3"} )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                                                        {a::saj_event::object_name_start, 0, "mmm1"},
                                                                                        {a::saj_event::string_value_start, 0, "wert"},
                                                                                        {a::saj_event::object_name_start, 0, "mem2"},
                                                                                        {a::saj_event::string_value_start, 0, "wert3"},
                                                                                        {a::saj_event::object_end}}));
}

TEST_CASE("Array : strings")
{
    char const                           input_buffer[] = R"( [ "m", "w", "m", "w"] )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::array_start},
                                                                                        {a::saj_event::string_value_start, 0, "m"},
                                                                                        {a::saj_event::string_value_start, 0, "w"},
                                                                                        {a::saj_event::string_value_start, 0, "m"},
                                                                                        {a::saj_event::string_value_start, 0, "w"},
                                                                                        {a::saj_event::array_end}}));
}
TEST_CASE("Array : error: comma seperation")
{
    char const                           input_buffer[] = R"( [ "m", "w", "m" "w"] )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::array_start},
                                                                                        {a::saj_event::string_value_start, 0, "m"},
                                                                                        {a::saj_event::string_value_start, 0, "w"},
                                                                                        {a::saj_event::string_value_start, 0, "m"},
                                                                                        {a::saj_event::parse_error, a::comma_expected}}));
}

TEST_CASE("Number: integer number")
{
    char const             input_buffer[] = R"( 901 )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::integer_value, 901}}));
}

TEST_CASE("Number: negative integer number")
{
    char const             input_buffer[] = R"( -21901 )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::integer_value, -21901}}));
}

TEST_CASE("Number: double number")
{
    char const             input_buffer[] = R"( 908.231 )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::float_value, 0, "", 908.231}}));
}

TEST_CASE("Number: negative double number")
{
    char const             input_buffer[] = R"( -0.1231231927 )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls,
                 Catch::Matchers::Equals(std::vector<call>{{a::saj_event::float_value, 0, "", -0.1231231927}}));
}

TEST_CASE("Number: exponent number")
{
    char const             input_buffer[] = R"( 1.2e-3 )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::float_value, 0, "", 1.2E-3}}));
}

TEST_CASE("Number: Exponent number")
{
    char const             input_buffer[] = R"( 1.2E-3 )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::float_value, 0, "", 1.2E-3}}));
}

TEST_CASE("Number: float number")
{
    char const                                             input_buffer[] = R"( 4721.32 )";
    a::basic_json_parser<test_handler<sp_float>, sp_float> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::float_value, 0, "", 4721.32f}}));
}

TEST_CASE("Array : integers")
{
    char const                           input_buffer[] = R"( [ 1, 3, -123, 12] )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::array_start},
                                                                                        {a::saj_event::integer_value, 1},
                                                                                        {a::saj_event::integer_value, 3},
                                                                                        {a::saj_event::integer_value, -123},
                                                                                        {a::saj_event::integer_value, 12},
                                                                                        {a::saj_event::array_end}}));
}

TEST_CASE("Array : doubles")
{
    char const                           input_buffer[] = R"( [ 1E-4, 12.4, -1.23, 12] )";
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::array_start},
                                                                                        {a::saj_event::float_value, 0, "", 1E-4},
                                                                                        {a::saj_event::float_value, 0, "", 12.4},
                                                                                        {a::saj_event::float_value, 0, "", -1.23},
                                                                                        {a::saj_event::integer_value, 12},
                                                                                        {a::saj_event::array_end}}));
}

TEST_CASE("partial input on text value")
{
    using namespace std::literals;
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(R"( { "blubber": "long te)"sv);
    p.parse_bytes(R"(xt string"}  )"sv);
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                                                        {a::saj_event::object_name_start, 0, "blubber"},
                                                                                        {a::saj_event::string_value_start, 0, "long text string"},
                                                                                        {a::saj_event::object_end}}));
}

TEST_CASE("partial input on text value 2")
{
    using namespace std::literals;
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(R"( { "blubber": "long text string)"sv);
    p.parse_bytes(R"("}  )"sv);
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                                                        {a::saj_event::object_name_start, 0, "blubber"},
                                                                                        {a::saj_event::string_value_start, 0, "long text string"},
                                                                                        {a::saj_event::object_end}}));
}

TEST_CASE("partial input on text value 3")
{
    using namespace std::literals;
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(R"( { "blubber": "long)"sv);
    p.parse_bytes(R"( text)"sv);
    p.parse_bytes(R"( string)"sv);
    p.parse_bytes(R"("  })"sv);
    REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                                                        {a::saj_event::object_name_start, 0, "blubber"},
                                                                                        {a::saj_event::string_value_start, 0, "long text string"},
                                                                                        {a::saj_event::object_end}}));
}

TEST_CASE("partial input on object name")
{
    using namespace std::literals;
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(R"( { "you )"sv);
    p.parse_bytes(R"(should not)"sv);
    p.parse_bytes(R"( create overlong object names -)"sv);
    p.parse_bytes(R"( but even if you do it will )"sv);
    p.parse_bytes(R"(work": 12  })"sv);
    REQUIRE_THAT(p.callback_handler()->calls,
                 Catch::Matchers::Equals(std::vector<call>{
                     {a::saj_event::object_start},
                     {a::saj_event::object_name_start, 0, "you should not create overlong object names - but even if you do it will work"},
                     {a::saj_event::integer_value, 12},
                     {a::saj_event::object_end}}));
}

TEST_CASE("escape sequences")
{
    using namespace std::literals;
    a::basic_json_parser<test_handler<>> p;
    p.parse_bytes(R"( { "str": "string with \" inside"})"sv);
    REQUIRE_THAT(p.callback_handler()->calls,
                 Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                           {a::saj_event::object_name_start, 0, "str"},
                                                           {a::saj_event::string_value_start, 0, R"(string with \" inside)"},
                                                           {a::saj_event::object_end}}));
}

