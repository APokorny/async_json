/* ==========================================================================
 Copyright (c) 2019-2022 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_BASIC_JSON_PARSER_HPP_INCLUDED
#define ASYNC_JSON_BASIC_JSON_PARSER_HPP_INCLUDED
#include <cmath>
#include <hsm/hsm.hpp>
#include <hsm/unroll_sm.hpp>
#include <async_json/default_traits.hpp>
#include <async_json/saj_event_value.hpp>
namespace async_json
{
struct unrolled_tag;
struct table_tag;

template <typename Handler = std::function<void(saj_event_value<default_traits> const&)>, typename Traits = default_traits, typename InterpreterTag = table_tag>
struct basic_json_parser
{
    using float_t   = typename Traits::float_t;
    using integer_t = typename Traits::integer_t;
    using sv_t      = typename Traits::sv_t;
    using event_value = saj_event_value<Traits>;

   private:
    Handler cbs;

    size_t byte_count{0};
    char   cur{0};

    // parsed states:
    struct keyword_receive
    {
        char const* kw{nullptr};
        uint8_t     pos{0};
    };
    keyword_receive kw_state;
    sv_t            parsed_view;
    sv_t            current_input_buffer;

    int                  num_sign{1};
    int                  exp_sign{1};
    int                  frac_digits{0};
    unsigned long long   exp_number{0};
    unsigned long long   int_number{0};
    unsigned long long   fraction{0};
    std::vector<uint8_t> state_stack;
    using self_t = basic_json_parser;
    std::function<bool(sv_t const&, int, self_t&)> process_events;

   private:
    auto get_fraction_we() -> float_t ; 
    auto get_number() -> integer_t;
    auto get_fraction() -> float_t;
    auto setup_sm() -> void;

   public:
    explicit basic_json_parser(Handler&& handler);
    basic_json_parser();

    auto callback_handler() { return &cbs; }
    auto parse_bytes(sv_t const& input) -> bool { return process_events(input, 0, *this); }
    auto reset() -> void;
};

#if defined(ASYNC_JSON_EXTERN) && !defined(ASYNC_JSON_EXPAND)
extern template class basic_json_parser<std::function<void(async_json::saj_event_value<async_json::default_traits> const& )>, default_traits, table_tag>;
#endif

}  // namespace async_json

#ifndef ASYNC_JSON_EXTERN
#include <async_json/detail/basic_json_parser.ipp>
#endif
#endif
