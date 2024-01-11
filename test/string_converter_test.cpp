#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <vector>
#include <functional>
#include <async_json/string_converter.hpp>
#include <catch2/catch.hpp>

namespace a = async_json;

TEST_CASE("StringConverter: json_to_utf8")
{
    using Catch::Matchers::Equals;
    REQUIRE_THAT(a::json_to_utf8("\\n"), Equals("\n"));
    REQUIRE_THAT(a::json_to_utf8("\\r"), Equals("\r"));
    REQUIRE_THAT(a::json_to_utf8("\\n\\n"), Equals("\n\n"));
    REQUIRE_THAT(a::json_to_utf8("\\t"), Equals("\t"));
    REQUIRE_THAT(a::json_to_utf8("\\b"), Equals("\b"));
    REQUIRE_THAT(a::json_to_utf8("\\\""), Equals("\""));
    REQUIRE_THAT(a::json_to_utf8("\\\\"), Equals("\\"));
    REQUIRE_THAT(a::json_to_utf8("\\\\\\\\"), Equals("\\\\"));
    REQUIRE_THAT(a::json_to_utf8("\\\\\\n"), Equals("\\\n"));
    REQUIRE_THAT(a::json_to_utf8("\\\\\\n\\\\\\n\\r\\t"), Equals("\\\n\\\n\r\t"));
    REQUIRE_THAT(a::json_to_utf8("d\\ne"), Equals("d\ne"));
    REQUIRE_THAT(a::json_to_utf8("a\\bnc"), Equals("a\bnc"));
    REQUIRE_THAT(a::json_to_utf8(" \\t "), Equals(" \t "));
    REQUIRE_THAT(a::json_to_utf8("abc def"), Equals("abc def"));
    REQUIRE_THAT(a::json_to_utf8("abc\\ud834\\udd20def"), Equals("abc㠴㌠def"));
    REQUIRE_THAT(a::json_to_utf8("mySTR\\\\\\nis\\tnice"), Equals("mySTR\\\nis\tnice"));
}

