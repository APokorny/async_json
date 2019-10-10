#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <iostream>
#include <iomanip>
#include <vector>
#include <functional>
#include <async_json/json_extractor.hpp>
#include "catch.hpp"

namespace a = async_json;
TEST_CASE("JSON Extractor: string value 1")
{
    constexpr char val[] = R"({ "var" : {"blub": "baz"}}  )";

    std::string foo;
    auto        extractor = a::make_extractor([](a::error_cause er) { std::cout << "ERROR" << static_cast<int>(er) << " \n"; },
                                       a::path(a::assign(foo, []() {}), "var", "blub"));
    extractor.parse_bytes(std::string_view(val, sizeof(val)));
    REQUIRE_THAT(foo, Catch::Matchers::Equals("baz"));
}

TEST_CASE("JSON Extractor: string value 2")
{
    constexpr char val[] = R"({ "var" : {"other": 12, "blub": "baz"}}  )";

    std::string foo;
    auto        extractor = a::make_extractor([](a::error_cause er) { std::cout << "ERROR" << static_cast<int>(er) << " \n"; },
                                       a::path(a::assign(foo, []() {}), "var", "blub"));
    extractor.parse_bytes(std::string_view(val, sizeof(val)));
    REQUIRE_THAT(foo, Catch::Matchers::Equals("baz"));
}

TEST_CASE("JSON Extractor: string value 3")
{
    constexpr char val[] = R"({ "var" : {"other": 12, "blub": "baz"}}  )";

    std::string foo;
    auto        extractor = a::make_extractor([](a::error_cause er) { std::cout << "ERROR" << static_cast<int>(er) << " \n"; },
                                       a::path(a::assign(foo, []() {}), "var", "blub"));
    extractor.parse_bytes(std::string_view(val, sizeof(val)));
    REQUIRE_THAT(foo, Catch::Matchers::Equals("baz"));
}
