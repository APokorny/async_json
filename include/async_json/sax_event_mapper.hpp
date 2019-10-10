/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_SAX_EVENT_MAPPER_H
#define ASYNC_JSON_SAX_EVENT_MAPPER_H

#include <async_json/basic_json_parser.hpp>

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
enum sax_event_id {
    sax_integer_value = 1,
    sax_boolean_value,
    sax_float_value,
    sax_null_value,
    sax_object_start,
    sax_object_end,
    sax_array_start,
    sax_array_end,
    sax_object_name_start,
    sax_object_name_cont,
    sax_object_name_end,
    sax_string_value_start,
    sax_string_value_cont,
    sax_string_value_end
};

template <typename... Ts>
constexpr auto create_sax_handler(Ts&&... ts) noexcept
{
    return hsm::create_state_machine(i_value, b_value, f_value, n_value, o_start, o_end, a_start, a_end, on_start, on_cont, on_end, str_start,
                                     str_cont, str_end, ts...);
}

// Utilty class to simplify building hsm based sax handlers
template <typename SMC, typename Traits = default_traits>
struct SaxEventMapper
{
    using sv_t      = typename Traits::sv_t;
    using integer_t = typename Traits::integer_t;
    using float_t   = typename Traits::float_t;

    sv_t      current_view;
    integer_t current_integer;
    float_t   current_float;
    bool      current_bool;

    SMC& cast() {return *static_cast<SMC*>(this); }

    template <typename S>
    constexpr auto matches(sv_t literal, S const& str)
    {
        return [&str, literal]() { return literal == str; };
    }
    constexpr auto begins_with(sv_t literal)
    {
        return [this, literal]() {
            if (literal.size() < current_view.size())
                return std::equal(begin(literal), end(literal), begin(current_view));
            else
                return std::equal(begin(current_view), end(current_view), begin(literal));
        };
    }

    template <typename S>
    constexpr auto store_string(S& ref)
    {
        return [this, &ref]() mutable { ref.append(begin(current_view), end(current_view)); };
    }

    template <typename S>
    constexpr auto clear(S& ref)
    {
        return [&ref]() mutable { ref.clear(); };
    }
    void value(bool v)
    {
        current_bool = v;
        cast().process_event(sax_boolean_value);
    }
    void value(float_t v)
    {
        current_float = v;
        cast().process_event(sax_float_value);
    }
    void value(integer_t v)
    {
        current_integer = v;
        cast().process_event(sax_integer_value);
    }
    void value(void*)
    {
        cast().process_event(sax_null_value);
    }
    void value(sv_t const& v)
    {
        current_view = v;
        cast().process_event(sax_string_value_start);
        current_view = sv_t{};
        cast().process_event(sax_string_value_end);
    }
    void string_value_start(sv_t const& v)
    {
        current_view = v;
        cast().process_event(sax_string_value_start);
    }
    void string_value_cont(sv_t const& v)
    {
        current_view = v;
        cast().process_event(sax_string_value_cont);
    }
    void string_value_end()
    {
        current_view = sv_t{};
        cast().process_event(sax_string_value_end);
    }
    void named_object(sv_t const& name)
    {
        current_view = name;
        cast().process_event(sax_object_name_start);
        current_view = sv_t{};
        cast().process_event(sax_object_name_end);
    }
    void named_object_start(sv_t const& name)
    {
        current_view = name;
        cast().process_event(sax_object_name_start);
    }
    void named_object_cont(sv_t const& name)
    {
        current_view = name;
        cast().process_event(sax_object_name_cont);
    }
    void named_object_end()
    {
        current_view = sv_t{};
        cast().process_event(sax_object_name_end);
    }
    void object_start() { cast().process_event(sax_object_start); }
    void object_end() { cast().process_event(sax_object_end); }
    void array_start() { cast().process_event(sax_array_start); }
    void array_end() { cast().process_event(sax_array_end); }
    void error(async_json::error_cause) {}
};
}  // namespace async_json

#endif
