/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_SAJ_EVENT_MAPPER_H
#define ASYNC_JSON_SAJ_EVENT_MAPPER_H

#include <async_json/basic_json_parser.hpp>
#include <async_json/saj_event_value.hpp>

namespace async_json
{
constexpr hsm::event<struct integer_value>      i_value;
constexpr hsm::event<struct boolean_value>      b_value;
constexpr hsm::event<struct float_value>        f_value;
constexpr hsm::event<struct null_value>         n_value;
constexpr hsm::event<struct object_start>       o_start;
constexpr hsm::event<struct object_end>         o_end;
constexpr hsm::event<struct array_start>        a_start;
constexpr hsm::event<struct array_end>          a_end;
constexpr hsm::event<struct object_name_start>  on_start;
constexpr hsm::event<struct object_name_cont>   on_cont;
constexpr hsm::event<struct object_name_end>    on_end;
constexpr hsm::event<struct string_value_start> str_start;
constexpr hsm::event<struct string_value_cont>  str_cont;
constexpr hsm::event<struct string_value_end>   str_end;
template <typename... Ts>
constexpr auto create_saj_state_machine(Ts&&... ts) noexcept
{
    return hsm::create_state_machine(i_value, b_value, f_value, n_value, o_start, o_end, a_start, a_end, on_start, on_cont, on_end,
                                     str_start, str_cont, str_end, std::forward<Ts>(ts)...);
}
/**
 * CRTP callback handler for the basic_json_parser, it calls a user provided
 * process_event(saj_event_id) method. The current value is provided through
 * the various current_ members. */
template <typename SMC, typename Traits = default_traits>
struct saj_event_mapper
{
    using sv_t      = typename Traits::sv_t;
    using integer_t = typename Traits::integer_t;
    using float_t   = typename Traits::float_t;
    using saj_value = saj_event_value<Traits>;

   public:  // a variant might be a nicer solution | needs to wait until C++17 is true for all my platforms
    saj_value event_value{saj_event::null_value};

    SMC& cast() { return *static_cast<SMC*>(this); }


    void value(bool v) { cast().process_event(event_value = saj_value(saj_event::boolean_value, v)); }
    void value(float_t v) { cast().process_event(event_value = saj_value(saj_event::float_value, v)); }
    void value(integer_t v) { cast().process_event(event_value = saj_value(saj_event::integer_value, v)); }
    void value(void*) { cast().process_event(event_value); }
    void value(sv_t const& v)
    {
        cast().process_event(event_value = saj_value(saj_event::string_value_start, v));
        cast().process_event(event_value = saj_value(saj_event::string_value_end));
    }
    void string_value_start(sv_t const& v) { cast().process_event(event_value = saj_value(saj_event::string_value_start, v)); }
    void string_value_cont(sv_t const& v) { cast().process_event(event_value = saj_value(saj_event::string_value_cont, v)); }
    void string_value_end() { cast().process_event(event_value = saj_value(saj_event::string_value_end)); }
    void named_object(sv_t const& name)
    {
        cast().process_event(event_value = saj_value(saj_event::object_name_start, name));
        cast().process_event(event_value = saj_value(saj_event::object_name_end));
    }
    void named_object_start(sv_t const& name) { cast().process_event(event_value = saj_value(saj_event::object_name_start, name)); }
    void named_object_cont(sv_t const& name) { cast().process_event(event_value = saj_value(saj_event::object_name_cont, name)); }
    void named_object_end() { cast().process_event(event_value = saj_value(saj_event::object_name_end)); }
    void object_start() { cast().process_event(event_value = saj_value(saj_event::object_start)); }
    void object_end() { cast().process_event(event_value = saj_value(saj_event::object_end)); }
    void array_start() { cast().process_event(event_value = saj_value(saj_event::array_start)); }
    void array_end() { cast().process_event(event_value = saj_value(saj_event::array_end)); }
    void error(async_json::error_cause err) { cast().process_event(event_value = saj_value(saj_event::parse_error, err)); }
};
}  // namespace async_json

#endif
