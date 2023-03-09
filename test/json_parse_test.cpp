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
    auto run_test = [](auto&& p)
    {
        char const input_buffer[] = "   null   ";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::null_value, 0}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Detect keywords: true")
{
    auto run_test = [](auto&& p)
    {
        char const input_buffer[] = "   true ";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::boolean_value, 1}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Detect keywords: false")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = " false ";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::boolean_value, 0}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Error in keyword: false")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = " fase ";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls,
                     Catch::Matchers::Equals(std::vector<call>{{a::saj_event::parse_error, a::wrong_keyword_character}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}
TEST_CASE("parse empty object")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( {} )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls,
                     Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start}, {a::saj_event::object_end}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}
TEST_CASE("Error: object expects member name")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( { [ }] )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls,
                     Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start}, {a::saj_event::parse_error, a::member_exp}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Parse named object")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( { "blubber": {} } )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                                                            {a::saj_event::object_name_start, 0, "blubber"},
                                                                                            {a::saj_event::object_start},
                                                                                            {a::saj_event::object_end},
                                                                                            {a::saj_event::object_end}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Parse empty array")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( { "blubber": [] } )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                                                            {a::saj_event::object_name_start, 0, "blubber"},
                                                                                            {a::saj_event::array_start},
                                                                                            {a::saj_event::array_end},
                                                                                            {a::saj_event::object_end}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Error unmatched array close")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( { "blubber": ] } )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls,
                     Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                               {a::saj_event::object_name_start, 0, "blubber"},
                                                               {a::saj_event::parse_error, a::unexpected_character}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Error object members must be named")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( { "blubber": {}, {} } )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                                                            {a::saj_event::object_name_start, 0, "blubber"},
                                                                                            {a::saj_event::object_start},
                                                                                            {a::saj_event::object_end},
                                                                                            {a::saj_event::parse_error, a::member_exp}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("String values")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( { "mmm1": "wert", "mem2": "wert3"} )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                                                            {a::saj_event::object_name_start, 0, "mmm1"},
                                                                                            {a::saj_event::string_value_start, 0, "wert"},
                                                                                            {a::saj_event::object_name_start, 0, "mem2"},
                                                                                            {a::saj_event::string_value_start, 0, "wert3"},
                                                                                            {a::saj_event::object_end}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Array : strings")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( [ "m", "w", "m", "w"] )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::array_start},
                                                                                            {a::saj_event::string_value_start, 0, "m"},
                                                                                            {a::saj_event::string_value_start, 0, "w"},
                                                                                            {a::saj_event::string_value_start, 0, "m"},
                                                                                            {a::saj_event::string_value_start, 0, "w"},
                                                                                            {a::saj_event::array_end}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Array : error: comma seperation")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( [ "m", "w", "m" "w"] )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls,
                     Catch::Matchers::Equals(std::vector<call>{{a::saj_event::array_start},
                                                               {a::saj_event::string_value_start, 0, "m"},
                                                               {a::saj_event::string_value_start, 0, "w"},
                                                               {a::saj_event::string_value_start, 0, "m"},
                                                               {a::saj_event::parse_error, a::comma_expected}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Number: integer number")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( 901 )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::integer_value, 901}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Number: negative integer number")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( -21901 )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::integer_value, -21901}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Number: double number")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( 908.231 )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::float_value, 0, "", 908.231}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Number: negative double number")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( -0.1231231927 )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls,
                     Catch::Matchers::Equals(std::vector<call>{{a::saj_event::float_value, 0, "", -0.1231231927}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Number: exponent number")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( 1.2e-3 )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::float_value, 0, "", 1.2E-3}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Number: Exponent number")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( 1.2E-3 )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::float_value, 0, "", 1.2E-3}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Number: float number")
{
    auto run_test = [](auto&& p)
    {
        char const                                             input_buffer[] = R"( 4721.32 )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::float_value, 0, "", 4721.32f}}));
    };

    run_test(a::basic_json_parser<test_handler<sp_float>, sp_float>{});
    run_test(a::basic_json_parser<test_handler<sp_float>, sp_float, a::unrolled_tag>{});
}

TEST_CASE("Array : integers")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( [ 1, 3, -123, 12] )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::array_start},
                                                                                            {a::saj_event::integer_value, 1},
                                                                                            {a::saj_event::integer_value, 3},
                                                                                            {a::saj_event::integer_value, -123},
                                                                                            {a::saj_event::integer_value, 12},
                                                                                            {a::saj_event::array_end}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("Array : doubles")
{
    auto run_test = [](auto&& p)
    {
        char const                           input_buffer[] = R"( [ 1E-4, 12.4, -1.23, 12] )";
        p.parse_bytes(std::string_view(input_buffer, sizeof(input_buffer)));
        REQUIRE_THAT(p.callback_handler()->calls, Catch::Matchers::Equals(std::vector<call>{{a::saj_event::array_start},
                                                                                            {a::saj_event::float_value, 0, "", 1E-4},
                                                                                            {a::saj_event::float_value, 0, "", 12.4},
                                                                                            {a::saj_event::float_value, 0, "", -1.23},
                                                                                            {a::saj_event::integer_value, 12},
                                                                                            {a::saj_event::array_end}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("partial input on text value")
{
    auto run_test = [](auto&& p)
    {
        using namespace std::literals;
        p.parse_bytes(R"( { "blubber": "long te)"sv);
        p.parse_bytes(R"(xt string"}  )"sv);
        REQUIRE_THAT(p.callback_handler()->calls,
                     Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                               {a::saj_event::object_name_start, 0, "blubber"},
                                                               {a::saj_event::string_value_start, 0, "long text string"},
                                                               {a::saj_event::object_end}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("partial input on text value 2")
{
    auto run_test = [](auto&& p)
    {
        using namespace std::literals;
        p.parse_bytes(R"( { "blubber": "long text string)"sv);
        p.parse_bytes(R"("}  )"sv);
        REQUIRE_THAT(p.callback_handler()->calls,
                     Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                               {a::saj_event::object_name_start, 0, "blubber"},
                                                               {a::saj_event::string_value_start, 0, "long text string"},
                                                               {a::saj_event::object_end}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("partial input on text value 3")
{
    auto run_test = [](auto&& p)
    {
        using namespace std::literals;
        p.parse_bytes(R"( { "blubber": "long)"sv);
        p.parse_bytes(R"( text)"sv);
        p.parse_bytes(R"( string)"sv);
        p.parse_bytes(R"("  })"sv);
        REQUIRE_THAT(p.callback_handler()->calls,
                     Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                               {a::saj_event::object_name_start, 0, "blubber"},
                                                               {a::saj_event::string_value_start, 0, "long text string"},
                                                               {a::saj_event::object_end}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("partial input on object name")
{
    auto run_test = [](auto&& p)
    {
        using namespace std::literals;
        p.parse_bytes(R"( { "you )"sv);
        p.parse_bytes(R"(should not)"sv);
        p.parse_bytes(R"( create overlong object names -)"sv);
        p.parse_bytes(R"( but even if you do it will )"sv);
        p.parse_bytes(R"(work": 12  })"sv);
        REQUIRE_THAT(
            p.callback_handler()->calls,
            Catch::Matchers::Equals(std::vector<call>{
                {a::saj_event::object_start},
                {a::saj_event::object_name_start, 0, "you should not create overlong object names - but even if you do it will work"},
                {a::saj_event::integer_value, 12},
                {a::saj_event::object_end}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

TEST_CASE("escape sequences")
{
    auto run_test = [](auto&& p)
    {
        using namespace std::literals;
        p.parse_bytes(R"( { "str": "string with \" inside"})"sv);
        REQUIRE_THAT(p.callback_handler()->calls,
                     Catch::Matchers::Equals(std::vector<call>{{a::saj_event::object_start},
                                                               {a::saj_event::object_name_start, 0, "str"},
                                                               {a::saj_event::string_value_start, 0, R"(string with \" inside)"},
                                                               {a::saj_event::object_end}}));
    };

    run_test(a::basic_json_parser<test_handler<>>{});
    run_test(a::basic_json_parser<test_handler<>, a::default_traits, a::unrolled_tag>{});
}

