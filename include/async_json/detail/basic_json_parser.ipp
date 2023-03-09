/* ==========================================================================
 Copyright (c) 2022 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_DETAIL_BASIC_JSON_PARSER_IPP_INCLUDED
#define ASYNC_JSON_DETAIL_BASIC_JSON_PARSER_IPP_INCLUDED
// #define ASYNC_JSON_PARSER_DEBUG
namespace async_json
{
namespace detail
{
constexpr hsm::event<struct character_event> ch;
constexpr hsm::event<struct digit_char>      digit;
constexpr hsm::event<struct quot_char>       quot;
constexpr hsm::event<struct colon_char>      colon;
constexpr hsm::event<struct comma_char>      comma;
constexpr hsm::event<struct dot_char>        dot;
constexpr hsm::event<struct exponent_char>   exponent;
constexpr hsm::event<struct plus_char>       plus;
constexpr hsm::event<struct minus_char>      minus;
constexpr hsm::event<struct br_open_char>    br_open;
constexpr hsm::event<struct br_close_char>   br_close;
constexpr hsm::event<struct idx_open_char>   idx_open;
constexpr hsm::event<struct idx_close_char>  idx_close;
constexpr hsm::event<struct ws>              whitespace;
constexpr hsm::event<struct t_char>          t;
constexpr hsm::event<struct f_char>          f;
constexpr hsm::event<struct n_char>          n;
constexpr hsm::event<struct esc_char>        escape;
constexpr hsm::event<struct end_of_input>    eoi;

constexpr hsm::state_ref<struct done_s>          done;
constexpr hsm::state_ref<struct error_s>         error;
constexpr hsm::state_ref<struct json_s>          json_state;
constexpr hsm::state_ref<struct json_in_array_s> json_state_in_array;
constexpr hsm::state_ref<struct keyword_s>       keyword;
constexpr hsm::state_ref<struct member_s>        member;

constexpr hsm::state_ref<struct string_start_cont_s>     string_start_cont;
constexpr hsm::state_ref<struct string_start_cont_esc_s> string_start_cont_esc;
constexpr hsm::state_ref<struct string_n_esc_s>          string_n_esc;
constexpr hsm::state_ref<struct string_n_s>              string_n;
constexpr hsm::state_ref<struct string_n_cont_s>         string_n_cont;
constexpr hsm::state_ref<struct string_n_cont_esc_s>     string_n_cont_esc;

constexpr hsm::state_ref<struct name_start_cont_s>     name_start_cont;
constexpr hsm::state_ref<struct name_start_cont_esc_s> name_start_cont_esc;
constexpr hsm::state_ref<struct name_n_s>              name_n;
constexpr hsm::state_ref<struct name_n_esc_s>          name_n_esc;
constexpr hsm::state_ref<struct name_n_cont_s>         name_n_cont;
constexpr hsm::state_ref<struct name_n_cont_esc_s>     name_n_cont_esc;

constexpr hsm::state_ref<struct int_number_s>      int_number_state;
constexpr hsm::state_ref<struct int_number_ws_s>   int_number_ws;
constexpr hsm::state_ref<struct fraction_number_s> fraction_number;
constexpr hsm::state_ref<struct exponent_sign_s>   exp_sign_state;
constexpr hsm::state_ref<struct exponent_s>        exp_state;
constexpr hsm::state_ref<struct expect_quot_s>     expect_quot;
constexpr hsm::state_ref<struct expect_colon_s>    expect_colon;

constexpr hsm::state_ref<struct array_object_comma_s>     array_object_comma;
constexpr hsm::state_ref<struct array_object_br_close_s>  array_object_br_close;
constexpr hsm::state_ref<struct array_object_idx_close_s> array_object_idx_close;
constexpr hsm::state_ref<struct array_object_s>           array_object;

template <error_cause err, typename S>
constexpr auto error_action()
{
    return [](S& self) { (*self.callback_handler())(typename S::event_value(async_json::saj_event::parse_error, err)); };
}
}  // namespace detail

template <typename Handler, typename Traits, typename IT>
auto basic_json_parser<Handler, Traits, IT>::get_fraction_we() -> float_t
{
    float_t ret = get_fraction() * static_cast<float_t>(std::pow(10, exp_sign * static_cast<integer_t>(exp_number)));
    exp_number  = 0;
    exp_sign    = 1;
    return ret;
}

template <typename Handler, typename Traits, typename IT>
auto basic_json_parser<Handler, Traits, IT>::get_number() -> integer_t
{
    integer_t ret = num_sign * static_cast<integer_t>(int_number);
    int_number    = 0;
    num_sign      = 1;
    return ret;
}
template <typename Handler, typename Traits, typename IT>
auto basic_json_parser<Handler, Traits, IT>::get_fraction() -> float_t
{
    float_t ret =
        num_sign * (static_cast<integer_t>(int_number) + static_cast<float_t>(fraction) / static_cast<float_t>(std::pow(10, frac_digits)));
    int_number  = 0;
    num_sign    = 1;
    fraction    = 0;
    frac_digits = 0;
    return ret;
}
template <typename Handler, typename Traits, typename IT>
auto basic_json_parser<Handler, Traits, IT>::setup_sm() -> void
{
    using namespace hsm::literals;
    auto kw_consume = [](self_t& self)
    { return self.cur == self.kw_state.kw[self.kw_state.pos] && 0 != self.kw_state.kw[self.kw_state.pos + 1]; };
    auto setup_true  = [](self_t& self) { self.kw_state = keyword_receive{"true", 1}; };
    auto setup_false = [](self_t& self) { self.kw_state = keyword_receive{"false", 1}; };
    auto setup_null  = [](self_t& self) { self.kw_state = keyword_receive{"null", 1}; };
    auto kw_complete = [](self_t& self)
    { return self.cur == self.kw_state.kw[self.kw_state.pos] && 0 == self.kw_state.kw[self.kw_state.pos + 1]; };
    auto kw_wrong_char       = [](self_t& self) { return self.cur != self.kw_state.kw[self.kw_state.pos]; };
    auto kw_consume_char     = [](self_t& self) { ++self.kw_state.pos; };
    auto kw_complete_keyword = [](self_t& self)
    {
        if (self.kw_state.kw == sv_t{"null"})
            self.cbs(event_value());
        else if (self.kw_state.kw == sv_t{"true"})
            self.cbs(event_value(saj_event::boolean_value, true));
        else
            self.cbs(event_value(saj_event::boolean_value, false));
    };

    auto negate_exp         = [](self_t& self) { self.exp_sign = -1; };
    auto negate_num         = [](self_t& self) { self.num_sign = -1; };
    auto add_digit_num      = [](self_t& self) { self.int_number = self.int_number * 10 + (self.cur - '0'); };
    auto add_digit_exp      = [](self_t& self) { self.exp_number = self.exp_number * 10 + (self.cur - '0'); };
    auto add_digit_fraction = [](self_t& self) { ++self.frac_digits, self.fraction = self.fraction * 10 + (self.cur - '0'); };

    auto emit_number       = [](self_t& self) { self.cbs(event_value(saj_event::integer_value, self.get_number())); };
    auto emit_fraction     = [](self_t& self) { self.cbs(event_value(saj_event::float_value, self.get_fraction())); };
    auto emit_exp_fraction = [](self_t& self) { self.cbs(event_value(saj_event::float_value, self.get_fraction_we())); };
    auto push_object       = [](self_t& self)
    {
        self.cbs(event_value(saj_event::object_start));
        self.state_stack.push_back(0);
    };
    auto pop_object = [](self_t& self)
    {
        self.cbs(event_value(saj_event::object_end));
        self.state_stack.pop_back();
    };
    auto push_array = [](self_t& self)
    {
        self.cbs(event_value(saj_event::array_start));
        self.state_stack.push_back(1);
    };
    auto pop_array = [](self_t& self)
    {
        self.cbs(event_value(saj_event::array_end));
        self.state_stack.pop_back();
    };
    auto emit_name_first = [](self_t& self)
    {
        self.cbs(event_value(saj_event::object_name_start, self.parsed_view));
        self.parsed_view = sv_t(nullptr, 0);
    };

    auto emit_name_first_last = [](self_t& self)
    {
        self.cbs(event_value(saj_event::object_name_start, self.parsed_view));
        self.parsed_view = sv_t(nullptr, 0);
        self.cbs(event_value(saj_event::object_name_end));
    };

    auto emit_name_n = [](self_t& self)
    {
        if (self.parsed_view.size()) self.cbs(event_value(saj_event::object_name_cont, self.parsed_view));
        self.parsed_view = sv_t(nullptr, 0);
    };

    auto emit_name_n_last = [](self_t& self)
    {
        if (self.parsed_view.size()) self.cbs(event_value(saj_event::object_name_cont, self.parsed_view));
        self.parsed_view = sv_t(nullptr, 0);
        self.cbs(event_value(saj_event::object_name_end));
    };

    auto emit_str_first = [](self_t& self)
    {
        self.cbs(event_value(saj_event::string_value_start, self.parsed_view));
        self.parsed_view = sv_t(nullptr, 0);
    };

    auto emit_str_first_last = [](self_t& self)
    {
        self.cbs(event_value(saj_event::string_value_start, self.parsed_view));
        self.cbs(event_value(saj_event::string_value_end));
        self.parsed_view = sv_t(nullptr, 0);
    };

    auto emit_str_n = [](self_t& self)
    {
        if (self.parsed_view.size()) self.cbs(event_value(saj_event::string_value_cont, self.parsed_view));
        self.parsed_view = sv_t(nullptr, 0);
    };

    auto emit_str_n_last = [](self_t& self)
    {
        if (self.parsed_view.size()) self.cbs(event_value(saj_event::string_value_cont, self.parsed_view));
        self.parsed_view = sv_t(nullptr, 0);
        self.cbs(event_value(saj_event::string_value_end));
    };

    auto stack_empty        = [](self_t& self) { return self.state_stack.size() == 0; };
    auto no_object_on_stack = [](self_t& self) { return self.state_stack.size() == 0 || self.state_stack.back() != 0; };
    auto object_on_stack    = [](self_t& self) { return self.state_stack.size() && self.state_stack.back() == 0; };
    auto no_array_on_stack  = [](self_t& self) { return self.state_stack.size() == 0 || self.state_stack.back() != 1; };
    auto array_on_stack     = [](self_t& self) { return self.state_stack.size() && self.state_stack.back() == 1; };
    auto mem_n_str          = [](self_t& self) { self.parsed_view = sv_t(self.current_input_buffer.begin(), 1); };
    auto mem_add_ch         = [](self_t& self)
    {
        self.parsed_view =
            sv_t(self.parsed_view.empty() ? self.current_input_buffer.begin() : self.parsed_view.begin(), self.parsed_view.size() + 1);
    };
    auto is_empty = [](self_t& self) { return self.parsed_view.empty(); };

    using namespace async_json::detail;
    auto select_sm = []<typename... Ts>(Ts&&... p) noexcept
    {
        if constexpr (std::is_same_v<IT, table_tag>) { return hsm::create_state_machine<self_t>(std::forward<Ts>(p)...); }
        else { return hsm::create_unrolled_sm<self_t>(std::forward<Ts>(p)...); }
    };
    auto sm = select_sm(  //
        ch,               // catch all event
        done,
        error,                                      //
        hsm::initial = json_state,                  //
        json_state(                                 //
            whitespace            = hsm::internal,  //
            n / setup_null        = keyword,
            f / setup_false       = keyword,              //
            t / setup_true        = keyword,              //
            br_open / push_object = member,               //
            idx_open / push_array = json_state_in_array,  //
            quot                  = string_start_cont,    //
            digit / add_digit_num = int_number_state,     //
            minus / negate_num    = int_number_ws,        //
            eoi                   = hsm::internal,        //
            json_state_in_array(                          //
                idx_close / pop_array = array_object),    //
            hsm::any / detail::error_action<unexpected_character, self_t>() = error),
        int_number_state(                                                                        //
            int_number_ws(                                                                       //
                whitespace            = hsm::internal,                                           //
                digit / add_digit_num = int_number_state),                                       //
            digit / add_digit_num                                     = hsm::internal,           //
            dot                                                       = fraction_number,         //
            exponent                                                  = exp_sign_state,          //
            comma / emit_number                                       = array_object_comma,      //
            br_close / emit_number                                    = array_object_br_close,   //
            idx_close / emit_number                                   = array_object_idx_close,  //
            whitespace / emit_number                                  = array_object,            //
            eoi                                                       = hsm::internal,           //
            hsm::any / detail::error_action<invalid_number, self_t>() = error),
        fraction_number(                                                                         //
            digit / add_digit_fraction                                = hsm::internal,           //
            exponent                                                  = exp_sign_state,          //
            comma / emit_fraction                                     = array_object_comma,      //
            br_close / emit_fraction                                  = array_object_br_close,   //
            idx_close / emit_fraction                                 = array_object_idx_close,  //
            whitespace / emit_fraction                                = array_object,            //
            eoi                                                       = hsm::internal,           //
            hsm::any / detail::error_action<invalid_number, self_t>() = error),
        exp_state(                                                                          //
            digit / add_digit_exp = hsm::internal,                                          //
            exp_sign_state(                                                                 //
                minus / negate_exp                                        = exp_state,      //
                plus / negate_exp                                         = exp_state,      //
                digit / add_digit_exp                                     = exp_state,      //
                eoi                                                       = hsm::internal,  //
                hsm::any / detail::error_action<invalid_number, self_t>() = error),
            comma / emit_exp_fraction                                 = array_object_comma,      //
            br_close / emit_exp_fraction                              = array_object_br_close,   //
            idx_close / emit_exp_fraction                             = array_object_idx_close,  //
            whitespace / emit_exp_fraction                            = array_object,            //
            eoi                                                       = hsm::internal,           //
            hsm::any / detail::error_action<invalid_number, self_t>() = error),
        string_start_cont(                                       //
            escape / mem_add_ch        = string_start_cont_esc,  //
            quot / emit_str_first_last = array_object,           //
            eoi[is_empty]              = hsm::internal,          //
            eoi / emit_str_first       = string_n,               //
            hsm::any / mem_add_ch      = string_start_cont),          //
        string_start_cont_esc(                                   //
            hsm::any / mem_add_ch = string_start_cont,           //
            eoi[is_empty]         = hsm::internal,               //
            eoi / emit_str_first  = string_n_esc),
        string_n(quot / emit_str_n_last = array_object,          //
                 escape / mem_n_str     = string_n_cont_esc,     //
                 hsm::any / mem_n_str   = string_n_cont),          //
        string_n_esc(hsm::any / mem_n_str = string_n_cont_esc),  //
        string_n_cont(                                           //
            hsm::any / mem_add_ch  = string_n_cont,              //
            escape / mem_add_ch    = string_n_cont_esc,          //
            quot / emit_str_n_last = array_object,               //
            eoi[is_empty]          = hsm::internal,              //
            eoi / emit_str_n       = string_n),
        string_n_cont_esc(                          //
            hsm::any / mem_add_ch = string_n_cont,  //
            eoi[is_empty]         = hsm::internal,  //
            eoi / emit_str_n      = string_n_esc),
        member(                                                                         //
            hsm::initial = expect_quot,                                                 //
            expect_quot(                                                                //
                whitespace                                            = hsm::internal,  //
                eoi                                                   = hsm::internal,
                quot                                                  = name_start_cont,  //
                br_close[object_on_stack] / pop_object                = array_object,     //
                hsm::any / detail::error_action<member_exp, self_t>() = error),
            name_start_cont(                                        //
                hsm::any / mem_add_ch       = name_start_cont,      //
                escape / mem_add_ch         = name_start_cont_esc,  //
                quot / emit_name_first_last = expect_colon,         //
                eoi[is_empty]               = hsm::internal,        //
                eoi / emit_name_first       = name_n),                    //
            name_start_cont_esc(                                    //
                hsm::any / mem_add_ch = name_start_cont,            //
                eoi[is_empty]         = hsm::internal,              //
                eoi / emit_name_first = name_n_esc),
            name_n(quot / emit_name_n_last = expect_colon,       //
                   escape / mem_n_str      = name_n_cont_esc,    //
                   hsm::any / mem_n_str    = name_n_cont),          //
            name_n_esc(hsm::any / mem_n_str = name_n_cont_esc),  //
            name_n_cont(                                         //
                hsm::any / mem_add_ch   = name_n_cont,           //
                escape / mem_add_ch     = name_n_cont_esc,       //
                quot / emit_name_n_last = expect_colon,          //
                eoi[is_empty]           = hsm::internal,         //
                eoi / emit_name_n       = name_n),
            name_n_cont_esc(                            //
                hsm::any / mem_add_ch = name_n_cont,    //
                eoi[is_empty]         = hsm::internal,  //
                eoi / emit_name_n     = name_n_esc)),
        expect_colon(                                                              //
            whitespace                                           = expect_colon,   //
            colon                                                = json_state,     //
            eoi                                                  = hsm::internal,  //
            hsm::any / detail::error_action<colon_exp, self_t>() = error),
        keyword(                                                                                                //
            eoi                                                                               = hsm::internal,  //
            hsm::any[kw_consume] / kw_consume_char                                            = keyword,
            hsm::any[kw_complete] / kw_complete_keyword                                       = array_object,  //
            hsm::any[kw_wrong_char] / detail::error_action<wrong_keyword_character, self_t>() = error          //
            ),
        array_object(                                   //
            hsm::initial[stack_empty] = done,           //
            whitespace                = hsm::internal,  //
            eoi                       = hsm::internal,  //
            comma                     = array_object_comma,
            array_object_comma(                             //
                hsm::initial[object_on_stack] = member,     //
                hsm::initial[array_on_stack]  = json_state  //
                ),
            br_close = array_object_br_close,
            array_object_br_close(                                                                            //
                hsm::initial[no_object_on_stack] / detail::error_action<mismatched_brace, self_t>() = error,  //
                hsm::initial[object_on_stack] / pop_object                                          = array_object),
            idx_close = array_object_idx_close,
            array_object_idx_close(                                                                          //
                hsm::initial[no_array_on_stack] / detail::error_action<mismatched_array, self_t>() = error,  //
                hsm::initial[array_on_stack] / pop_array                                           = array_object),
            hsm::any / detail::error_action<comma_expected, self_t>() = error  //
            ));
    sm.start(*this);
    process_events = [sm = std::move(sm)](sv_t const& bytes, int ctrl, self_t& self) mutable
    {
        if (ctrl < 0) sm.start(self);
        self.current_input_buffer = bytes;
#ifdef ASYNC_JSON_PARSER_DEBUG
        auto to_state_name = [](int d) -> char const*
        {
            char const* fpp[] = {"root",
                                 "done",
                                 "error",
                                 "json_state",
                                 "json_state_in_array",
                                 "int_number_state",
                                 "int_number_ws",
                                 "fraction_number",
                                 "esp_state",
                                 "exp_sign_state",
                                 "string_start_cont",
                                 "string_start_cont_esc",
                                 "string_n",
                                 "string_n_esc",
                                 "string_n_cont",
                                 "string_n_cont_esc",
                                 "member",
                                 "expect_quot",
                                 "name_star_cont",
                                 "name_start_cont_esc",
                                 "name_n",
                                 "name_n_esc",
                                 "name_n_cont",
                                 "name_n_cont_esc",
                                 "expect_colon",
                                 "keyword",
                                 "array_object",
                                 "array_object_comma",
                                 "array_object_br_close",
                                 "array_object_idx_close"};
            return fpp[d];
        };
#endif
        auto switch_char = [&sm](char c, self_t& s) mutable
        {
            switch (c)
            {
                case 'n': return sm.process_event(n, s);
                case 't': return sm.process_event(t, s);
                case 'f': return sm.process_event(f, s);
                case '{': return sm.process_event(br_open, s);
                case '}': return sm.process_event(br_close, s);
                case '[': return sm.process_event(idx_open, s);
                case ']': return sm.process_event(idx_close, s);
                case '"': return sm.process_event(quot, s);
                case ':': return sm.process_event(colon, s);
                case ',': return sm.process_event(comma, s);
                case '+': return sm.process_event(plus, s);
                case '-': return sm.process_event(minus, s);
                case '0': /* fall-through */
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': return sm.process_event(digit, s);
                case '\\': return sm.process_event(escape, s);
                case 'E':
                case 'e': return sm.process_event(exponent, s);
                case '.': return sm.process_event(dot, s);
                case ' ':
                case '\b':
                case '\t':
                case '\r':
                case '\n': return sm.process_event(whitespace, s);
                default: return sm.process_event(ch, s);
            }
        };
        for (auto elem : bytes)
        {
            self.cur = elem;
#ifdef ASYNC_JSON_PARSER_DEBUG
            std::cout << &self << "Parse: '" << self.cur << "' " << to_state_name(static_cast<int>(sm.current_state_id())) << " ";
            for (auto const& entry : self.state_stack) std::cout << int(entry) << " ";
            std::cout << "\n";
#endif
            switch_char(self.cur, self);
            if (sm.current_state_id() == sm.get_state_id(error)) return false;
            ++self.byte_count;
            self.current_input_buffer = sv_t(self.current_input_buffer.begin() + 1, self.current_input_buffer.size() - 1);
        }
        sm.process_event(eoi, self);
        return sm.current_state_id() != sm.get_state_id(error);
    };
}

template <typename Handler, typename Traits, typename IT>
basic_json_parser<Handler, Traits, IT>::basic_json_parser(Handler&& handler) : cbs(std::move(handler))
{
    setup_sm();
}

template <typename Handler, typename Traits, typename IT>
basic_json_parser<Handler, Traits, IT>::basic_json_parser()
{
    setup_sm();
}

template <typename Handler, typename Traits, typename IT>
auto basic_json_parser<Handler, Traits, IT>::reset() -> void
{
    exp_sign    = 1;
    frac_digits = 0;
    exp_number  = 0;
    int_number  = 0;
    fraction    = 0;
    byte_count  = 0;
    process_events(sv_t{}, -1, *this);
}
}  // namespace async_json
#endif
