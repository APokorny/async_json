/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_UTILS_H
#define ASYNC_JSON_UTILS_H

#include <algorithm>

namespace async_json
{
template <typename SvT, typename S>
constexpr auto matches(SvT literal, S const& str)
{
    return [&str, literal]() {
        return literal == str;
    };
}

template <typename S, typename T>
constexpr auto begins_with(S literal, saj_event_value<T>& ev)
{
    return [&ev, literal]() {
        if (literal.size() < ev.as_string_view().size())
            return std::equal(begin(literal), end(literal), begin(ev.as_string_view()));
        else
            return std::equal(begin(ev.as_string_view()), end(ev.as_string_view()), begin(literal));
    };
}

template <typename S, typename T>
constexpr auto store_string(S& ref, saj_event_value<T>& ev)
{
    return [&ev, &ref]() mutable { ref.append(begin(ev.as_string_view()), end(ev.as_string_view())); };
}

template <typename S>
constexpr auto clear(S& ref)
{
    return [&ref]() mutable { ref.clear(); };
}
}  // namespace async_json

#endif
