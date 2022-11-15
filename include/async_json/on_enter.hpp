/* ==========================================================================
 Copyright (c) 2022 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_ON_ENTER_HPP_INCLUDED
#define ASYNC_JSON_ON_ENTER_HPP_INCLUDED

namespace async_json
{
template<typename C>
constexpr auto on_enter(C &&c) noexcept
{
    return [amplitude = -1, c](auto const& ev) mutable
    {
        if (amplitude == -1 && (ev.event == async_json::saj_event::object_start || ev.event == async_json::saj_event::array_start))
            c(ev);
        if (ev.event == async_json::saj_event::object_start || ev.event == async_json::saj_event::array_start)
            ++amplitude;
        else if (ev.event == async_json::saj_event::object_end || ev.event == async_json::saj_event::array_end)
            --amplitude;
    };
}
}  // namespace async_json

#endif
