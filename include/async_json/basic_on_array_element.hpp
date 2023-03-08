/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_BASIC_ON_ARRAY_ELEMENT_HPP_INCLUDED
#define ASYNC_JSON_BASIC_ON_ARRAY_ELEMENT_HPP_INCLUDED

#include <async_json/saj_event_mapper.hpp>
#include <async_json/default_traits.hpp>

namespace async_json
{
struct table_tag;
struct unrolled_tag;
template <typename Traits, typename InterpreterTag = async_json::table_tag>
struct basic_on_array_element
{
    using event_value = async_json::saj_event_value<Traits>;

    std::function<void()>                                            callback;
    std::function<bool(basic_on_array_element*, event_value const&)> ev_handler;

    bool event_ret{false};
    bool event_has_value{false};
    int  depth_counter{0};

    template <typename C>
    basic_on_array_element(C&& fun) : callback(std::forward<C>(fun))
    {
        setup_sm();
    }
    void setup_sm();
    void operator()(event_value const&);
};

#if defined(ASYNC_JSON_EXTERN) && !defined(ASYNC_JSON_EXPAND)
extern template class basic_on_array_element<default_traits>;
#endif
}  // namespace async_json
#ifndef ASYNC_JSON_EXTERN
#include <async_json/detail/basic_on_array_element.ipp>
#endif
#endif
