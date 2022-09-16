#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <iostream>
#include <iomanip>
#include <vector>
#include <functional>
#include <async_json/json_extractor.hpp>
#include <catch2/catch.hpp>

namespace a = async_json;
TEST_CASE("JSON Extractor: string value 1")
{
    constexpr char val[] = R"({ "var" : {"blub": "baz"}}  )";

    std::string foo;
    auto        extractor = a::make_extractor([](a::error_cause er) { std::cout << "ERROR" << static_cast<int>(er) << " \n"; },
                                       a::path(a::assign_string(foo), "var", "blub"));
    extractor.parse_bytes(std::string_view(val, sizeof(val)));
    REQUIRE_THAT(foo, Catch::Matchers::Equals("baz"));
}

TEST_CASE("JSON Extractor: string value 2")
{
    constexpr char val[] = R"({ "var" : {"other": 12, "blub": "baz"}}  )";

    std::string foo;
    auto        extractor = a::make_extractor([](a::error_cause er) { std::cout << "ERROR" << static_cast<int>(er) << " \n"; },
                                       a::path(a::assign_string(foo), "var", "blub"));
    extractor.parse_bytes(std::string_view(val, sizeof(val)));
    REQUIRE_THAT(foo, Catch::Matchers::Equals("baz"));
}

TEST_CASE("JSON Extractor: string value 3")
{
    constexpr char val[] = R"({ "var" : {"other": 12, "blub": "baz"}}  )";

    std::string foo;
    auto        extractor = a::make_extractor([](a::error_cause er) { std::cout << "ERROR" << static_cast<int>(er) << " \n"; },
                                       a::path(a::assign_string(foo), "var", "blub"));
    extractor.parse_bytes(std::string_view(val, sizeof(val)));
    REQUIRE_THAT(foo, Catch::Matchers::Equals("baz"));
}

TEST_CASE("JSON Path: int value 1")
{
    constexpr char val[] = R"({"bar":{ "foo": 5}})";
    long           foo;
    auto           extractor = a::make_extractor([](a::error_cause er) { std::cout << "ERROR" << static_cast<int>(er) << " \n"; },
                                       a::path(a::assign_numeric(foo), "bar", "foo"));
    extractor.parse_bytes(std::string_view(val, sizeof(val)));
    REQUIRE(foo == 5);
}

TEST_CASE("JSON Path: int value 2")
{
    constexpr char val[]     = R"({"bar":{ "not_foo": { "other_thing": 10}, "foo": 10}})";
    long           foo       = 12;
    auto           extractor = a::make_extractor([](a::error_cause er) { std::cout << "ERROR" << static_cast<int>(er) << " \n"; },
                                       a::path(a::assign_numeric(foo), "bar", "foo"));
    extractor.parse_bytes(std::string_view(val, sizeof(val)));
    REQUIRE(foo == 10);
}

TEST_CASE("JSON Path: in array")
{
    constexpr char val[]     = R"([{"bar":{ "foo": 0}},{"bar":{ "foo": 1}}] )";
    long           foo       = 0;
    auto           extractor = a::make_extractor([](a::error_cause er) { std::cout << "ERROR" << static_cast<int>(er) << " \n"; },
                                       a::path(a::assign_numeric(foo), "bar", "foo"));
    extractor.parse_bytes(std::string_view(val, sizeof(val)));
    REQUIRE(foo == 1);
}

TEST_CASE("JSON Path: side by side")
{
    constexpr char val[]     = R"({"bar": 0,"foo": 0} )";
    long           foo       = 13;
    auto           extractor = a::make_extractor([](a::error_cause er) { std::cout << "ERROR" << static_cast<int>(er) << " \n"; },
                                       a::path(a::assign_numeric(foo), "bar", "foo"));
    extractor.parse_bytes(std::string_view(val, sizeof(val)));
    REQUIRE(foo == 13);
}

TEST_CASE("JSON Path: side by side in array")
{
    constexpr char val[]     = R"([{"bar": {},"foo": {}}] )";
    long           foo       = 13;
    auto           extractor = a::make_extractor([](a::error_cause er) { std::cout << "ERROR" << static_cast<int>(er) << " \n"; },
                                       a::path(a::assign_numeric(foo), "bar", "foo"));
    extractor.parse_bytes(std::string_view(val, sizeof(val)));
    REQUIRE(foo == 13);
}

TEST_CASE("JSON Path: arbitrary")
{
    constexpr char val[]     = R"({"bar":{"foo": 31}} )";
    long           foo       = 13;
    auto           extractor = a::make_extractor([](a::error_cause er) { std::cout << "ERROR" << static_cast<int>(er) << " \n"; },
                                       a::path(a::assign_numeric(foo), "bar", a::arbitrary));
    extractor.parse_bytes(std::string_view(val, sizeof(val)));
    REQUIRE(foo == 31);
}
