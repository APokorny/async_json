/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_SAJ_EVENT_MAPPER_H
#define ASYNC_JSON_SAJ_EVENT_MAPPER_H

#include <async_json/basic_json_parser.hpp>
#include <async_json/saj_event_value.hpp>
#include <hsm/unroll_sm.hpp>

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
template <typename C, typename InterpreterTag, typename... Ts>
constexpr inline auto create_saj_state_machine(Ts&&... ts) noexcept
{
    if constexpr (std::is_same_v<InterpreterTag, table_tag>)
    {
        return hsm::create_state_machine<C>(n_value, i_value, b_value, f_value, o_start, o_end, a_start, a_end, on_start, on_cont,
                                                 on_end,

                                                 std::forward<Ts>(ts)...);
    }
    else
    {
        return hsm::create_unrolled_sm<C>(n_value, i_value, b_value, f_value, o_start, o_end, a_start, a_end, on_start, on_cont, on_end,
                                          str_start, str_cont, str_end, std::forward<Ts>(ts)...);
    }
}

}  // namespace async_json

#endif
