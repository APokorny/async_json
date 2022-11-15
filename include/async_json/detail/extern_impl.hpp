/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_DETAIL_EXTERN_IMPL_HPP_INCLUDED
#define ASYNC_JSON_DETAIL_EXTERN_IMPL_HPP_INCLUDED

#define ASYNC_JSON_EXPAND
#include <async_json/basic_is_path.hpp>
#include <async_json/detail/basic_is_path.ipp>
#include <async_json/basic_on_array_element.hpp>
#include <async_json/detail/basic_on_array_element.ipp>
#include <async_json/basic_json_parser.hpp>
#include <async_json/detail/basic_json_parser.ipp>

template class async_json::basic_is_path<async_json::default_traits>;
template class async_json::basic_on_array_element<async_json::default_traits>;
template class async_json::basic_json_parser<std::function<void(async_json::saj_event_value<async_json::default_traits> const&)>, async_json::default_traits>;
#endif
