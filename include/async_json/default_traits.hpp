/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_DEFAULT_TRAITS_HPP_INCLUDED
#define ASYNC_JSON_DEFAULT_TRAITS_HPP_INCLUDED

// get string view:

#if __cplusplus >= 201703L
#include <string_view>
#else
#include <experimental/string_view>
namespace std
{
using string_view = experimental::basic_string_view<char>;
}
#endif

namespace async_json
{
enum error_cause
{
    no_error                = 0,
    wrong_keyword_character = 1,
    mismatched_array,
    mismatched_brace,
    member_exp,
    colon_exp,
    unexpected_character,
    invalid_number,
    comma_expected
};

struct default_traits
{
    using float_t   = double;
    using integer_t = long;
    using sv_t      = std::string_view;
};

}  // namespace async_json

#endif
