/* ==========================================================================
 Copyright (c) 2022 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_ON_EXIT_HPP_INCLUDED
#define ASYNC_JSON_ON_EXIT_HPP_INCLUDED
#include <async_json/saj_event_value.hpp>

namespace async_json
{

template <typename C>
constexpr auto on_exit(C&& c) noexcept
{
    return [amplitude = 0, c](auto const& ev) mutable 
    {
        if (amplitude >= 0)
        {
            if (ev.event == async_json::saj_event::object_start)
                ++amplitude;
            else if (ev.event == async_json::saj_event::object_end)
                if (--amplitude == 0) c();
        }
        if (amplitude <= 0)
        {
            if (ev.event == async_json::saj_event::array_start)
                --amplitude;
            else if (ev.event == async_json::saj_event::array_end)
                if (++amplitude == 0) c();
        }
    };
}
}  // namespace async_json

#endif
