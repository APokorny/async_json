/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_SAJ_EVENT_VALUE_H
#define ASYNC_JSON_SAJ_EVENT_VALUE_H

#include <type_traits>
#include <algorithm>
#include <variant>
#include <async_json/default_traits.hpp>  // error cause

namespace async_json
{
struct void_t
{
};

enum class saj_variant_value : uint8_t
{
    none         = 0,
    number       = 1 << 4,
    float_number = 2 << 4,
    boolean      = 3 << 4,
    string       = 4 << 4,
    error        = 5 << 4,
    mask         = 0xF0
};

template <typename E>
constexpr std::underlying_type_t<E> cast(E v) noexcept
{
    return static_cast<std::underlying_type_t<E>>(v);
}
enum class saj_event : uint8_t
{
    null_value         = 1 + cast(saj_variant_value::none),
    integer_value      = 2 + cast(saj_variant_value::number),
    boolean_value      = 3 + cast(saj_variant_value::boolean),
    float_value        = 4 + cast(saj_variant_value::float_number),
    object_start       = 5 + cast(saj_variant_value::none),
    object_end         = 6 + cast(saj_variant_value::none),
    array_start        = 7 + cast(saj_variant_value::none),
    array_end          = 8 + cast(saj_variant_value::none),
    object_name_start  = 9 + cast(saj_variant_value::string),
    object_name_cont   = 10 + cast(saj_variant_value::string),
    object_name_end    = 11 + cast(saj_variant_value::none),
    string_value_start = 12 + cast(saj_variant_value::string),
    string_value_cont  = 13 + cast(saj_variant_value::string),
    string_value_end   = 14 + cast(saj_variant_value::none),
    parse_error        = 15 + cast(saj_variant_value::error)
};

template <typename Traits>
struct saj_event_value
{
    using sv_t      = typename Traits::sv_t;
    using integer_t = typename Traits::integer_t;
    using float_t   = typename Traits::float_t;

    saj_event event{saj_event::null_value};
    std::variant<sv_t, bool, float_t, integer_t, error_cause> store;

    constexpr saj_event_value()                        = default;
    constexpr saj_event_value(saj_event_value const &) = default;
    constexpr saj_event_value(saj_event_value &&)      = default;
    constexpr saj_event_value &operator=(saj_event_value const &) = default;
    constexpr saj_event_value &operator=(saj_event_value &&) = default;
    constexpr explicit saj_event_value(saj_event ev) : event{ev} {}
    constexpr saj_event_value(saj_event ev, float_t f) : event{ev}, store(f) {}
    constexpr saj_event_value(saj_event ev, integer_t i) : event{ev}, store(i) {}
    constexpr saj_event_value(saj_event ev, bool b) : event{ev}, store(b) {}
    constexpr saj_event_value(saj_event ev, error_cause e) : event{ev}, store(e) {}
    constexpr saj_event_value(saj_event ev, sv_t s) : event{ev}, store(s) {}

    constexpr auto as_number() const noexcept { return std::get<integer_t>(store); }
    constexpr auto as_float_number() const noexcept { return std::get<float_t>(store); }
    constexpr auto as_string_view() const noexcept { return std::get<sv_t>(store); }
    constexpr auto as_bool() const noexcept { return std::get<bool>(store); }
    constexpr auto as_error_cause() const noexcept { return std::get<error_cause>(store); }
    constexpr bool is_value() const noexcept
    {
        switch (event)
        {
            case saj_event::null_value:
            case saj_event::integer_value:
            case saj_event::boolean_value:
            case saj_event::float_value:
            case saj_event::string_value_start:
            case saj_event::string_value_cont:
            case saj_event::string_value_end: return true;
            default: return false;
        }
    }
    constexpr bool has_value() const noexcept { return (cast(event) & cast(saj_variant_value::mask)) != cast(saj_variant_value::none); }
    constexpr saj_variant_value value_type() const noexcept
    {
        return static_cast<saj_variant_value>(cast(event) & cast(saj_variant_value::mask));
    }
    constexpr auto as_event_id() const noexcept { return cast(event) & 0xF; }
};

}  // namespace async_json
#endif
