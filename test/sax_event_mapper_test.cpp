#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <iostream>
#include <iomanip>
#include <vector>
#include <functional>
#include <async_json/sax_event_mapper.hpp>
#include "catch.hpp"

namespace a = async_json;

TEST_CASE("SaxEventMapper: test")
{
    using namespace std::literals;
    struct Stuff : a::SaxEventMapper<Stuff>
    {
        // TODO Put this in the class decl?
        std::function<void(uint8_t)> process_event;

        Stuff()
        {
            auto sm       = async_json::create_sax_handler();
            sm.start();
            process_event = [this, sm = std::move(sm)](uint8_t b) mutable {
                switch (b)
                {
                    case a::sax_integer_value: return sm.process_event(a::i_value);
                    case a::sax_boolean_value: return sm.process_event(a::b_value);
                    case a::sax_float_value: return sm.process_event(a::f_value);
                    case a::sax_null_value: return sm.process_event(a::n_value);
                    case a::sax_object_start: return sm.process_event(a::o_start);
                    case a::sax_object_end: return sm.process_event(a::o_end);
                    case a::sax_array_start: return sm.process_event(a::a_start);
                    case a::sax_array_end: return sm.process_event(a::a_end);
                    case a::sax_object_name_start: return sm.process_event(a::on_start);
                    case a::sax_object_name_cont: return sm.process_event(a::on_cont);
                    case a::sax_object_name_end: return sm.process_event(a::on_end);
                    case a::sax_string_value_start: return sm.process_event(a::str_start);
                    case a::sax_string_value_cont: return sm.process_event(a::str_cont);
                    case a::sax_string_value_end: return sm.process_event(a::str_end);
                }
            };
        }
    };

    a::basic_json_parser<Stuff> p;
    p.parse_bytes(R"( { "you": 123, "a": {} }  )"sv);
}

