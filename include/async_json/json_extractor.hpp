/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_JSON_EXTRACTOR_HPP_INCLUDED
#define ASYNC_JSON_JSON_EXTRACTOR_HPP_INCLUDED

#include <async_json/basic_json_parser.hpp>
#include <async_json/is_path.hpp>
#include <memory>
#include <type_traits>

namespace async_json
{
namespace detail
{
template <typename Traits, typename T, typename EH>
struct value_handler;

template <typename Traits, typename T, typename EH>
struct value_handler : default_handler<Traits>
{
    using integer_t = typename Traits::integer_t;
    using sv_t      = typename Traits::sv_t;
    using float_t   = typename Traits::float_t;
    T&   destination;
    EH&  error_handler;
    bool astart{false};
    value_handler(T& dest, EH&& eh) : destination{dest}, error_handler{eh} {}
    template <typename VT>
    void value(VT const& vt, typename std::enable_if<std::is_convertible<VT, T>::value>::type* /*ptr*/ = nullptr)
    {
        destination = vt;
    }
    template <typename VT>
    void value(VT const&, typename std::enable_if<!std::is_convertible<VT, T>::value>::type* /*ptr*/ = nullptr)
    {
        error_handler();
    }
    void string_value_start(sv_t const&) { error_handler(); }
    void string_value_cont(sv_t const&) { error_handler(); }
    void string_value_end() { error_handler(); }
    void array_start() { error_handler(); }
    void array_end() { error_handler(); }
};

template <typename Traits, typename T, typename A, typename EH>
struct value_handler<Traits, std::vector<T, A>, EH> : default_handler<Traits>
{
    using integer_t = typename Traits::integer_t;
    using sv_t      = typename Traits::sv_t;
    using float_t   = typename Traits::float_t;
    T&   destination;
    EH&  error_handler;
    bool astart{false};
    value_handler(std::vector<T, A>& dest, EH&& eh) : destination{dest}, error_handler{eh} {}
    template <typename VT>
    void value(VT const& vt, typename std::enable_if<std::is_convertible<VT, T>::value>::type* /*ptr*/ = nullptr)
    {
        destination.push_back(vt);
    }
    template <typename VT>
    void value(VT const&, typename std::enable_if<!std::is_convertible<VT, T>::value>::type* /*ptr*/ = nullptr)
    {
        error_handler();
    }
    void string_value_start(sv_t const&) { error_handler(); }
    void string_value_cont(sv_t const&) { error_handler(); }
    void string_value_end() { error_handler(); }
    void array_start() { astart = true; }
    void array_end() { astart = false; }
};

template <typename Traits, typename A, typename EH>
struct value_handler<Traits, std::vector<std::string, A>, EH> : default_handler<Traits>
{
    using integer_t = typename Traits::integer_t;
    using sv_t      = typename Traits::sv_t;
    using float_t   = typename Traits::float_t;
    std::vector<std::string, A>& destination;
    EH                           error_handler;
    bool                         astart{false};
    value_handler(std::vector<std::string, A>& dest, EH&& eh) : destination{dest}, error_handler{eh} {}
    template <typename VT>
    void value(VT const&)
    {
        error_handler();
    }
    void value(sv_t const& str)
    {
        if (astart) destination.emplace_back(str.begin(), str.end());
    }
    void string_value_start(sv_t const& s)
    {
        if (astart) destination.emplace_back(s.begin(), s.end());
    }
    void string_value_cont(sv_t const& s)
    {
        if (astart) destination.back().append(s.begin(), s.end());
    }
    void string_value_end() {}
    void array_start() { astart = true; }
    void array_end() { astart = false; }
};

template <typename Traits, typename EH>
struct value_handler<Traits, std::string, EH> : default_handler<Traits>
{
    using integer_t = typename Traits::integer_t;
    using sv_t      = typename Traits::sv_t;
    using float_t   = typename Traits::float_t;
    std::string& destination;
    EH           error_handler;
    value_handler(std::string& dest, EH&& eh) : destination{dest}, error_handler{eh} {}
    using default_handler<Traits>::value;
    void value(sv_t const& str) { destination.assign(str.begin(), str.end()); }
    void string_value_start(sv_t const& s) { destination.assign(s.begin(), s.end()); }
    void string_value_cont(sv_t const& s) { destination.append(s.begin(), s.end()); }
    void string_value_end() {}
    void array_start() {}
    void array_end() {}
};

template <typename Traits, typename EH, typename... Ts>
struct extractor : saj_event_mapper<extractor<Traits, EH, Ts...>, Traits>
{
    tiny_tuple::tuple<Ts...> data;
    EH                       error_handler;
    constexpr extractor(EH&& eh, Ts&&... ts) noexcept : data{ts...}, error_handler{eh} {}
    using sv_t      = typename Traits::sv_t;
    using integer_t = typename Traits::integer_t;
    using float_t   = typename Traits::float_t;
    using ev_t      = saj_event_value<Traits>;
    void process_event(ev_t const& ev)
    {
        if (ev.event != saj_event::parse_error)
            tiny_tuple::foreach (data, [&ev](auto& i) { i(ev); });
        else
            error_handler(ev.as_error_cause());
    }
};

}  // namespace detail

template <typename T>
constexpr auto assign_numeric(T& ref)
{
    return [&ref](auto const& ev) {
        switch (ev.value_type())
        {
            case saj_variant_value::float_number: ref = static_cast<T>(ev.as_float_number()); break;
            case saj_variant_value::number: ref = static_cast<T>(ev.as_number()); break;
            case saj_variant_value::boolean: ref = static_cast<T>(ev.as_bool()); break;
            // case saj_variant_value::string: number_from_sv_t::try_parse(ev.as_string_view(), ref); break;
            default: break;
        }
    };
}

template <typename T>
constexpr auto assign_string(T& ref)
{
    return [&ref](auto const& ev) {
        if (ev.event == saj_event::string_value_start) ref.assign(ev.as_string_view().begin(), ev.as_string_view().end());
        if (ev.event == saj_event::string_value_cont) ref.append(ev.as_string_view().begin(), ev.as_string_view().end());
    };
}

template <typename T>
constexpr auto assign_name(T& ref)
{
    return [&ref](auto const& ev) {
        if (ev.event == saj_event::object_name_start) ref.assign(ev.as_string_view().begin(), ev.as_string_view().end);
        if (ev.event == saj_event::object_name_cont) ref.append(ev.as_string_view().begin(), ev.as_string_view().end);
    };
}

template <typename A, typename... Ts>
constexpr auto path(A&& a, Ts&&... ts) noexcept
{
    return [path_test = async_json::is_path(detail::path_element(std::forward<Ts>(ts))...), a](auto ev) mutable {
        if (path_test(ev)) a(ev);
    };
}

template <typename EH, typename... Ts>
constexpr auto make_extractor(EH&& eh, Ts&&... ts) noexcept
{
    return basic_json_parser<detail::extractor<default_traits, EH, Ts...>>(
        detail::extractor<default_traits, EH, Ts...>(std::forward<EH>(eh), std::forward<Ts>(ts)...));
}

template <typename OtherTraits, typename EH, typename... Ts>
constexpr auto make_extractor(EH&& eh, Ts&&... ts) noexcept
{
    return basic_json_parser<detail::extractor<OtherTraits, EH, Ts...>>(
        detail::extractor<OtherTraits, EH, Ts...>(std::forward<EH>(eh), std::forward<Ts>(ts)...));
}

}  // namespace async_json

#endif
