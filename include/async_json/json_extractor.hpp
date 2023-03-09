/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_JSON_EXTRACTOR_HPP_INCLUDED
#define ASYNC_JSON_JSON_EXTRACTOR_HPP_INCLUDED

#include <async_json/basic_json_parser.hpp>
#include <async_json/basic_is_path.hpp>
#include <async_json/basic_on_array_element.hpp>
#include <memory>
#include <vector>
#include <variant>
#include <type_traits>

namespace async_json
{
namespace detail
{

template <typename Traits, typename EH, typename... Ts>
struct extractor
{
    tiny_tuple::tuple<Ts...> data;
    EH                       error_handler;
    constexpr extractor(EH&& eh, Ts&&... ts) noexcept : data{ts...}, error_handler{eh} {}
    using sv_t      = typename Traits::sv_t;
    using integer_t = typename Traits::integer_t;
    using float_t   = typename Traits::float_t;
    using ev_t      = saj_event_value<Traits>;
    void operator()(ev_t const& ev)
    {
        if (ev.event != saj_event::parse_error)
            tiny_tuple::foreach (data, [&ev](auto& i) { i(ev); });
        else
            error_handler(ev.as_error_cause());
    }
};

}  // namespace detail

template <typename T>
struct is_container
{
    constexpr static bool value = false;
};
template <typename... Ts>
struct is_container<std::vector<Ts...>>
{
    constexpr static bool value = true;
};

template <typename... Ts>
inline constexpr auto all(Ts&&... ts) noexcept
{
    return [funs = tiny_tuple::tuple<std::decay_t<Ts>...>(std::forward<Ts>(ts)...)](auto const& ev) mutable
    { tiny_tuple::foreach (funs, [&](auto& item) { item(ev); }); };
}

template <typename T>
constexpr auto assign_numeric(T& ref, std::enable_if_t<!is_container<T>::value>* = nullptr)
{
    return [&ref](auto const& ev)
    {
        switch (ev.value_type())
        {
            case saj_variant_value::float_number: ref = ev.as_float_number(); break;
            case saj_variant_value::number: ref = ev.as_number(); break;
            case saj_variant_value::boolean: ref = ev.as_bool(); break;
            // case saj_variant_value::string: number_from_sv_t::try_parse(ev.as_string_view(), ref); break;
            default: break;
        }
    };
}

template <typename F>
constexpr auto assign_numeric(F&& fun, std::enable_if_t<!is_container<std::decay_t<decltype(fun())>>::value>* = nullptr)
{
    return [fun](auto const& ev)
    {
        using dest = std::decay_t<decltype(fun())>;
        switch (ev.value_type())
        {
            case saj_variant_value::float_number: fun() = ev.as_float_number(); break;
            case saj_variant_value::number: fun() = ev.as_number(); break;
            case saj_variant_value::boolean: fun() = ev.as_bool(); break;
            // case saj_variant_value::string: number_from_sv_t::try_parse(ev.as_string_view(), ref); break;
            default: break;
        }
    };
}

template <typename T>
constexpr auto assign_numeric(T& ref, std::enable_if_t<is_container<T>::value>* = nullptr)
{
    return [&ref](auto const& ev)
    {
        switch (ev.value_type())
        {
            case saj_variant_value::float_number: ref.push_back(ev.as_float_number()); break;
            case saj_variant_value::number: ref.push_back(ev.as_number()); break;
            case saj_variant_value::boolean: ref.push_back(ev.as_bool()); break;
            // case saj_variant_value::string: number_from_sv_t::try_parse(ev.as_string_view(), ref); break;
            default: break;
        }
    };
}

template <typename F>
constexpr auto assign_numeric(F&& fun, std::enable_if_t<is_container<std::decay_t<decltype(fun())>>::value>* = nullptr)
{
    return [fun](auto const& ev)
    {
        using value_type = typename std::decay_t<decltype(fun())>::value_type;
        switch (ev.value_type())
        {
            case saj_variant_value::float_number: fun().push_back(ev.as_float_number()); break;
            case saj_variant_value::number: fun().push_back(ev.as_number()); break;
            case saj_variant_value::boolean: fun().push_back(ev.as_bool()); break;
            // case saj_variant_value::string: number_from_sv_t::try_parse(ev.as_string_view(), ref); break;
            default: break;
        }
    };
}

template <typename T>
constexpr auto assign_string(T& ref, std::enable_if_t<!is_container<T>::value>* = nullptr)
{
    return [&ref](auto const& ev)
    {
        if (ev.event == saj_event::string_value_start) ref.assign(ev.as_string_view().begin(), ev.as_string_view().end());
        if (ev.event == saj_event::string_value_cont) ref.append(ev.as_string_view().begin(), ev.as_string_view().end());
    };
}

template <typename... Ts>
constexpr auto assign_string(std::variant<Ts...>& ref)
{
    return [&ref](auto const& ev)
    {
        if (ev.event == saj_event::string_value_start) ref = std::string(ev.as_string_view().begin(), ev.as_string_view().end());
        if (ev.event == saj_event::string_value_cont)
            std::get<std::string>(ref).append(ev.as_string_view().begin(), ev.as_string_view().end());
    };
}

template <typename T>
constexpr auto assign_string(T& ref, std::enable_if_t<is_container<T>::value>* = nullptr)
{
    return [&ref](auto const& ev)
    {
        if (ev.event == saj_event::string_value_start) ref.push_back(T::value_type(ev.as_string_view().begin(), ev.as_string_view().end()));
        if (ev.event == saj_event::string_value_cont) ref.back().append(ev.as_string_view().begin(), ev.as_string_view().end());
    };
}

template <typename F>
constexpr auto assign_string(F&& fun, std::enable_if_t<!is_container<std::decay_t<decltype(fun())>>::value>* = nullptr)
{
    return [fun](auto const& ev)
    {
        if (ev.event == saj_event::string_value_start) fun().assign(ev.as_string_view().begin(), ev.as_string_view().end());
        if (ev.event == saj_event::string_value_cont) fun().append(ev.as_string_view().begin(), ev.as_string_view().end());
    };
}

template <typename F>
constexpr auto assign_string(F&& fun, std::enable_if_t<is_container<std::decay_t<decltype(fun())>>::value>* = nullptr)
{
    using value_type = typename std::decay_t<decltype(fun())>::value_type;
    return [fun](auto const& ev)
    {
        if (ev.event == saj_event::string_value_start) fun().push_back(value_type(ev.as_string_view().begin(), ev.as_string_view().end()));
        if (ev.event == saj_event::string_value_cont) fun().back().append(ev.as_string_view().begin(), ev.as_string_view().end());
    };
}

template <typename T>
constexpr auto assign_name(T& ref)
{
    return [&ref](auto const& ev)
    {
        if (ev.event == saj_event::object_name_start) ref.assign(ev.as_string_view().begin(), ev.as_string_view().end());
        if (ev.event == saj_event::object_name_cont) ref.append(ev.as_string_view().begin(), ev.as_string_view().end());
    };
}

template <typename A, typename Traits, typename IT = table_tag>
struct basic_path
{
    basic_is_path<Traits, IT> is_path;
    A                         fun;
    template <typename... Ts>
    basic_path(A&& a, Ts&&... ts) : fun(a), is_path({static_cast<detail::path_element>(ts)...})
    {
    }
    void operator()(async_json::saj_event_value<Traits> ev) noexcept
    {
        if (is_path(ev)) fun(ev);
    }
};

template <typename A, typename... Ts>
constexpr auto path(A&& a, Ts&&... ts) noexcept
{
    return basic_path<A, default_traits>(std::forward<A>(a), std::forward<Ts>(ts)...);
}

template <typename A, typename... Ts>
constexpr auto fast_path(A&& a, Ts&&... ts) noexcept
{
    return basic_path<A, default_traits, unrolled_tag>(std::forward<A>(a), std::forward<Ts>(ts)...);
}

using on_array_element      = basic_on_array_element<default_traits>;
using fast_on_array_element = basic_on_array_element<default_traits, unrolled_tag>;

template <typename EH, typename... Ts>
constexpr auto make_extractor(EH&& eh, Ts&&... ts) noexcept
{
    return basic_json_parser<>(detail::extractor<default_traits, EH, Ts...>(std::forward<EH>(eh), std::forward<Ts>(ts)...));
}

template <typename EH, typename... Ts>
constexpr auto make_fast_extractor(EH&& eh, Ts&&... ts) noexcept
{
    return basic_json_parser<std::function<void(async_json::saj_event_value<default_traits> const&)>, default_traits, unrolled_tag>(
        detail::extractor<default_traits, EH, Ts...>(std::forward<EH>(eh), std::forward<Ts>(ts)...));
}

template <typename OtherTraits, typename EH, typename... Ts>
constexpr auto make_extractor(EH&& eh, Ts&&... ts) noexcept
{
    return basic_json_parser<std::function<void(async_json::saj_event_value<OtherTraits> const&)>, OtherTraits>(
        detail::extractor<OtherTraits, EH, Ts...>(std::forward<EH>(eh), std::forward<Ts>(ts)...));
}

template <typename OtherTraits, typename EH, typename... Ts>
constexpr auto make_fast_extractor(EH&& eh, Ts&&... ts) noexcept
{
    return basic_json_parser<std::function<void(async_json::saj_event_value<OtherTraits> const&)>, OtherTraits, unrolled_tag>(
        detail::extractor<OtherTraits, EH, Ts...>(std::forward<EH>(eh), std::forward<Ts>(ts)...));
}

}  // namespace async_json

#endif
