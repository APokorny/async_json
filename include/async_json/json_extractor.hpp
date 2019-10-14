/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_JSON_EXTRACTOR_HPP_INCLUDED
#define ASYNC_JSON_JSON_EXTRACTOR_HPP_INCLUDED

#include <async_json/basic_json_parser.hpp>
#include <memory>
#include <type_traits>

namespace async_json
{
namespace detail
{
template <typename Traits, typename T, typename... Ts>
struct path : async_json::default_handler<Traits>
{
    using integer_t = typename Traits::integer_t;
    using sv_t      = typename Traits::sv_t;
    using float_t   = typename Traits::float_t;
    std::array<sv_t, sizeof...(Ts)> path_elements;
    size_t                          off_path{0};
    bool                            on_path{true};
    size_t                          element{0};
    size_t                          pos{0};
    T                               destination;
    path(T&& dest, Ts&&... ts) : path_elements{{ts...}}, destination{dest} {}
    template <typename VT>
    void value(VT const& v)
    {
        if (on_path && element == sizeof...(Ts)) destination.value(v);
    }
    void string_value_start(sv_t const& v)
    {
        if (on_path && element == sizeof...(Ts)) destination.string_value_start(v);
    }
    void string_value_cont(sv_t const& v)
    {
        if (on_path && element == sizeof...(Ts)) destination.string_value_start(v);
    }
    void string_value_end()
    {
        if (on_path && element == sizeof...(Ts)) destination.string_value_end();
    }
    bool begins_with(sv_t const& str, sv_t const& b)
    {
        auto i1 = str.begin();
        auto e1 = str.end();
        auto i2 = b.begin();
        auto e2 = b.end();
        for (; i1 != e1 && i2 != e2; ++i1, ++i2)
            if (*i1 != *i2) return false;
        return i2 == e2;
    }
    void named_object(sv_t const& s)
    {
        if (off_path == 0 && begins_with(path_elements[element], s) && s.size() == path_elements[element].size())
        {
            ++element;
            pos     = 0;
            on_path = true;
        }
        else
        {
            on_path = false;
        }
    }
    void named_object_start(sv_t const& s)
    {
        if (off_path == 0 && begins_with(path_elements[element], s))
            pos += s.size();
        else
            on_path = false;
    }
    void named_object_cont(sv_t const& s)
    {
        if (pos != 0 && begins_with(path_elements[element].substr(pos), s))
            pos += s.size();
        else
            on_path = false;
    }
    void named_object_end()
    {
        if (pos == path_elements[element].size())
        {
            ++element;
            on_path = true;
        }
        pos = 0;
    }
    void object_start()
    {
        if (!on_path) ++off_path;
    }
    void object_end()
    {
        if (off_path) --off_path;
        if (0 == off_path) on_path = true;
    }
};

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
struct extractor
{
    tiny_tuple::tuple<Ts...> data;
    EH                       error_handler;
    constexpr extractor(EH&& eh, Ts&&... ts) noexcept : data{ts...}, error_handler{eh} {}

    using sv_t      = typename Traits::sv_t;
    using integer_t = typename Traits::integer_t;
    using float_t   = typename Traits::float_t;
    void value(void*) {}
    template <typename T>
    void value(T const& v)
    {
        tiny_tuple::foreach (data, [v](auto& i) { i.value(v); });
    }
    void string_value_start(sv_t const& s)
    {
        tiny_tuple::foreach (data, [&s](auto& i) { i.string_value_start(s); });
    }
    void string_value_cont(sv_t const& s)
    {
        tiny_tuple::foreach (data, [&s](auto& i) { i.string_value_cont(s); });
    }
    void string_value_end()
    {
        tiny_tuple::foreach (data, [](auto& i) { i.string_value_end(); });
    }
    void named_object(sv_t const& name)
    {
        tiny_tuple::foreach (data, [&name](auto& i) { i.named_object(name); });
    }
    void named_object_start(sv_t const& name)
    {
        tiny_tuple::foreach (data, [&name](auto& i) { i.named_object_start(name); });
    }
    void named_object_cont(sv_t const& name)
    {
        tiny_tuple::foreach (data, [&name](auto& i) { i.named_object_cont(name); });
    }
    void named_object_end()
    {
        tiny_tuple::foreach (data, [](auto& i) { i.named_object_end(); });
    }
    void object_start()
    {
        tiny_tuple::foreach (data, [](auto& i) { i.object_start(); });
    }
    void object_end()
    {
        tiny_tuple::foreach (data, [](auto& i) { i.object_end(); });
    }
    void array_start()
    {
        tiny_tuple::foreach (data, [](auto& i) { i.array_start(); });
    }
    void array_end()
    {
        tiny_tuple::foreach (data, [](auto& i) { i.array_end(); });
    }
    void error(error_cause cause) { error_handler(cause); }
};

}  // namespace detail

template <typename T, typename EH>
constexpr auto assign(T& dest, EH&& error) noexcept
{
    return detail::value_handler<default_traits, T, EH>(dest, std::forward<EH>(error));
}

template <typename Traits, typename T, typename EH>
constexpr auto assign(T& dest, EH&& error) noexcept
{
    return detail::value_handler<Traits, T, EH>(dest, std::forward<EH>(error));
}

template <typename Traits, typename A, typename... Ts>
constexpr auto path(A&& a, Ts&&... ts) noexcept
{
    return detail::path<Traits, A, Ts...>(std::forward<A>(a), std::forward<Ts>(ts)...);
}

template <typename A, typename... Ts>
constexpr auto path(A&& a, Ts&&... ts) noexcept
{
    return detail::path<default_traits, A, Ts...>(std::forward<A>(a), std::forward<Ts>(ts)...);
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
