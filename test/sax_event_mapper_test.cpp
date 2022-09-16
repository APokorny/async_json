#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <iostream>
#include <iomanip>
#include <vector>
#include <functional>
#include <async_json/saj_event_mapper.hpp>
#include <catch2/catch.hpp>

namespace a = async_json;

TEST_CASE("SajEventMapper: test")
{
    using namespace std::literals;
    struct Stuff : a::saj_event_mapper<Stuff>
    {
        // TODO Put this in the class decl?
        std::function<void(a::saj_event_value<a::default_traits>)> process_event;

        Stuff()
        {
            auto sm = async_json::create_saj_state_machine<Stuff>();
            sm.start(*this);
            process_event = [this, sm = std::move(sm)](a::saj_event_value<a::default_traits> b) mutable {
                switch (b.event)
                {
                    case a::saj_event::integer_value: return sm.process_event(a::i_value, *this);
                    case a::saj_event::boolean_value: return sm.process_event(a::b_value, *this);
                    case a::saj_event::float_value: return sm.process_event(a::f_value, *this);
                    case a::saj_event::null_value: return sm.process_event(a::n_value, *this);
                    case a::saj_event::object_start: return sm.process_event(a::o_start, *this);
                    case a::saj_event::object_end: return sm.process_event(a::o_end, *this);
                    case a::saj_event::array_start: return sm.process_event(a::a_start, *this);
                    case a::saj_event::array_end: return sm.process_event(a::a_end, *this);
                    case a::saj_event::object_name_start: return sm.process_event(a::on_start, *this);
                    case a::saj_event::object_name_cont: return sm.process_event(a::on_cont, *this);
                    case a::saj_event::object_name_end: return sm.process_event(a::on_end, *this);
                    case a::saj_event::string_value_start: return sm.process_event(a::str_start, *this);
                    case a::saj_event::string_value_cont: return sm.process_event(a::str_cont, *this);
                    case a::saj_event::string_value_end: return sm.process_event(a::str_end, *this);
                    case a::saj_event::parse_error:
                    default: return false;
                }
            };
        }
    };

    a::basic_json_parser<Stuff> p;
    p.parse_bytes(R"( { "you": 123, "a": {} }  )"sv);
}

