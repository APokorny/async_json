/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_BASIC_JSON_PARSER_HPP_INCLUDED
#define ASYNC_JSON_BASIC_JSON_PARSER_HPP_INCLUDED
#include <cmath>
#include <hsm/hsm.hpp>
#include <async_json/default_traits.hpp>

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
constexpr hsm::event<struct end_of_input>    eoi;

constexpr hsm::state_ref<struct done_s>    done;
constexpr hsm::state_ref<struct error_s>   error;
constexpr hsm::state_ref<struct json_s>    json_state;
constexpr hsm::state_ref<struct keyword_s> keyword;
constexpr hsm::state_ref<struct member_s>  member;

constexpr hsm::state_ref<struct string_start_s>      string_start;
constexpr hsm::state_ref<struct string_start_cont_s> string_start_cont;
constexpr hsm::state_ref<struct string_start_ch_s>   string_start_ch;
constexpr hsm::state_ref<struct string_n_s>          string_n;
constexpr hsm::state_ref<struct string_n_cont_s>     string_n_cont;
constexpr hsm::state_ref<struct string_n_ch_s>       string_n_ch;

constexpr hsm::state_ref<struct name_start_s>      name_start;
constexpr hsm::state_ref<struct name_start_cont_s> name_start_cont;
constexpr hsm::state_ref<struct name_start_ch_s>   name_start_ch;
constexpr hsm::state_ref<struct name_n_s>          name_n;
constexpr hsm::state_ref<struct name_n_cont_s>     name_n_cont;
constexpr hsm::state_ref<struct name_n_ch_s>       name_n_ch;

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
}  // namespace detail

template <typename Traits>
struct default_handler
{
    using sv_t      = typename Traits::sv_t;
    using integer_t = typename Traits::integer_t;
    using float_t   = typename Traits::float_t;
    void value(bool) {}
    void value(void*) {}
    void value(integer_t) {}
    void value(float_t) {}
    void value(sv_t const&) {}
    void string_value_start(sv_t const&) {}
    void string_value_cont(sv_t const&) {}
    void string_value_end() {}
    void named_object(sv_t const&) {}
    void named_object_start(sv_t const&) {}
    void named_object_cont(sv_t const&) {}
    void named_object_end() {}
    void object_start() {}
    void object_end() {}
    void array_start() {}
    void array_end() {}
    void error(error_cause) {}
};

template <typename Handler = default_handler<default_traits>, typename Traits = default_traits>
struct basic_json_parser
{
    using float_t   = typename Traits::float_t;
    using integer_t = typename Traits::integer_t;
    using sv_t      = typename Traits::sv_t;

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

    int                                   num_sign{1};
    int                                   exp_sign{1};
    int                                   frac_digits{0};
    unsigned long long                    exp_number{0};
    unsigned long long                    int_number{0};
    unsigned long long                    fraction{0};
    std::vector<uint8_t>                  state_stack;
    std::function<bool(sv_t const&, int)> process_events;

    void setup_sm()
    {
        using namespace hsm::literals;
        auto* cb                  = this->callback_handler();
        auto  kw_consume          = [this]() { return cur == kw_state.kw[kw_state.pos] && 0 != kw_state.kw[kw_state.pos + 1]; };
        auto  setup_kw            = [this](char const* name) { return [this, name]() { kw_state = keyword_receive{name, 1}; }; };
        auto  kw_complete         = [this]() { return cur == kw_state.kw[kw_state.pos] && 0 == kw_state.kw[kw_state.pos + 1]; };
        auto  kw_wrong_char       = [this]() { return cur != kw_state.kw[kw_state.pos]; };
        auto  kw_consume_char     = [this]() { ++kw_state.pos; };
        auto  kw_complete_keyword = [cb, this]() {
            if (kw_state.kw == sv_t{"null"})
                cb->value(nullptr);
            else if (kw_state.kw == sv_t{"true"})
                cb->value(true);
            else
                cb->value(false);
        };

        auto negate      = [](auto& sign) { return [&sign]() { sign = -1; }; };
        auto add_digit   = [this](auto& num) { return [&num, this]() { num = num * 10 + (cur - '0'); }; };
        auto add_digit_c = [this](auto& num, auto& count) { return [&num, &count, this]() { ++count, num = num * 10 + (cur - '0'); }; };
        auto get_number  = [this]() {
            integer_t ret = num_sign * static_cast<integer_t>(int_number);
            int_number    = 0;
            num_sign      = 1;
            return ret;
        };
        auto get_fraction = [this]() {
            float_t ret = num_sign * (static_cast<integer_t>(int_number) +
                                      static_cast<float_t>(fraction) / static_cast<float_t>(std::pow(10, frac_digits)));
            int_number  = 0;
            num_sign    = 1;
            fraction    = 0;
            frac_digits = 0;
            return ret;
        };
        auto get_fraction_we = [get_fraction, this]() {
            float_t ret = get_fraction() * static_cast<float_t>(std::pow(10, exp_sign * static_cast<integer_t>(exp_number)));
            exp_number  = 0;
            exp_sign    = 1;
            return ret;
        };

        auto emit_number       = [cb, get_number]() { cb->value(get_number()); };
        auto emit_fraction     = [cb, get_fraction]() { cb->value(get_fraction()); };
        auto emit_exp_fraction = [cb, get_fraction_we]() { cb->value(get_fraction_we()); };
        auto push_object       = [cb, this]() {
            cb->object_start();
            state_stack.push_back(0);
        };
        auto pop_object = [cb, this]() {
            cb->object_end();
            state_stack.pop_back();
        };
        auto push_array = [cb, this]() {
            cb->array_start();
            state_stack.push_back(1);
        };
        auto pop_array = [cb, this]() {
            cb->array_end();
            state_stack.pop_back();
        };
        auto emit_name_first = [cb, this]() {
            cb->named_object_start(parsed_view);
            parsed_view = sv_t(nullptr, 0);
        };

        auto emit_name_first_last = [cb, this]() {
            cb->named_object(parsed_view);
            parsed_view = sv_t(nullptr, 0);
        };

        auto emit_name_n = [cb, this]() {
            if (parsed_view.size()) cb->named_object_cont(parsed_view);
            parsed_view = sv_t(nullptr, 0);
        };

        auto emit_name_n_last = [cb, emit_name_n]() {
            emit_name_n();
            cb->named_object_end();
        };

        auto emit_str_first = [cb, this]() {
            cb->string_value_start(parsed_view);
            parsed_view = sv_t(nullptr, 0);
        };

        auto emit_str_first_last = [cb, this]() {
            cb->value(parsed_view);
            parsed_view = sv_t(nullptr, 0);
        };

        auto emit_str_n = [cb, this]() {
            if (parsed_view.size()) cb->string_value_cont(parsed_view);
            parsed_view = sv_t(nullptr, 0);
        };

        auto emit_str_n_last = [cb, emit_str_n]() {
            emit_str_n();
            cb->string_value_end();
        };

        auto error_action       = [cb, this](error_cause reason) { return [reason, cb, this]() { cb->error(reason); }; };
        auto stack_empty        = [this]() { return state_stack.size() == 0; };
        auto no_object_on_stack = [this]() { return state_stack.size() == 0 || state_stack.back() != 0; };
        auto object_on_stack    = [this]() { return state_stack.size() && state_stack.back() == 0; };
        auto no_array_on_stack  = [this]() { return state_stack.size() == 0 || state_stack.back() != 1; };
        auto array_on_stack     = [this]() { return state_stack.size() && state_stack.back() == 1; };
        auto mem_start_str      = [this]() { parsed_view = sv_t(current_input_buffer.begin(), 1); };
        auto mem_add_ch         = [this]() { parsed_view = sv_t(parsed_view.begin(), parsed_view.size() + 1); };

        using namespace async_json::detail;
        auto sm = hsm::create_state_machine(  //
            ch,                               // catch all event
            done,
            error,                                                              //
            hsm::initial = json_state,                                          //
            json_state(                                                         //
                whitespace                                    = hsm::internal,  //
                n / setup_kw("null")                          = keyword,
                f / setup_kw("false")                         = keyword,           //
                t / setup_kw("true")                          = keyword,           //
                br_open / push_object                         = member,            //
                idx_open / push_array                         = json_state,        //
                quot                                          = string_start,      //
                digit / add_digit(int_number)                 = int_number_state,  //
                minus / negate(num_sign)                      = int_number_ws,     //
                eoi                                           = hsm::internal,     //
                hsm::any / error_action(unexpected_character) = error),
            int_number_state(                                                      //
                int_number_ws(                                                     //
                    whitespace                    = hsm::internal,                 //
                    digit / add_digit(int_number) = int_number_state),             //
                digit / add_digit(int_number)           = hsm::internal,           //
                dot                                     = fraction_number,         //
                exponent                                = exp_sign_state,          //
                comma / emit_number                     = array_object_comma,      //
                br_close / emit_number                  = array_object_br_close,   //
                idx_close / emit_number                 = array_object_idx_close,  //
                whitespace / emit_number                = array_object,            //
                eoi                                     = hsm::internal,           //
                hsm::any / error_action(invalid_number) = error),
            fraction_number(                                                          //
                digit / add_digit_c(fraction, frac_digits) = hsm::internal,           //
                exponent                                   = exp_sign_state,          //
                comma / emit_fraction                      = array_object_comma,      //
                br_close / emit_fraction                   = array_object_br_close,   //
                idx_close / emit_fraction                  = array_object_idx_close,  //
                whitespace / emit_fraction                 = array_object,            //
                eoi                                        = hsm::internal,           //
                hsm::any / error_action(invalid_number)    = error),
            exp_state(                                                        //
                digit / add_digit(exp_number) = hsm::internal,                //
                exp_sign_state(                                               //
                    minus / negate(exp_sign)                = exp_state,      //
                    plus / negate(exp_sign)                 = exp_state,      //
                    digit / add_digit(exp_number)           = exp_state,      //
                    eoi                                     = hsm::internal,  //
                    hsm::any / error_action(invalid_number) = error),
                comma / emit_exp_fraction               = array_object_comma,      //
                br_close / emit_exp_fraction            = array_object_br_close,   //
                idx_close / emit_exp_fraction           = array_object_idx_close,  //
                whitespace / emit_exp_fraction          = array_object,            //
                eoi                                     = hsm::internal,           //
                hsm::any / error_action(invalid_number) = error),
            string_start(  //
                hsm::any / mem_start_str = string_start_cont,
                string_start_ch(                                    //
                    hsm::any / mem_add_ch = hsm::internal,          //
                    string_start_cont(                              //
                        quot / emit_str_first_last = array_object,  //
                        eoi / emit_str_first       = string_n))),
            string_n(                                     //
                quot / emit_str_n_last   = array_object,  //
                hsm::any / mem_start_str = string_n_cont,
                string_n_ch(                                    //
                    hsm::any / mem_add_ch = hsm::internal,      //
                    string_n_cont(                              //
                        quot / emit_str_n_last = array_object,  //
                        eoi / emit_str_n       = string_n))),
            member(                                                          //
                hsm::initial = expect_quot,                                  //
                expect_quot(                                                 //
                    whitespace                             = hsm::internal,  //
                    eoi                                    = hsm::internal,
                    quot                                   = name_start,    //
                    br_close[object_on_stack] / pop_object = array_object,  //
                    hsm::any / error_action(member_exp)    = error),
                name_start(  //
                    hsm::any / mem_start_str = name_start_cont,
                    name_start_ch(                                       //
                        hsm::any / mem_add_ch = hsm::internal,           //
                        name_start_cont(                                 //
                            quot / emit_name_first_last = expect_colon,  //
                            eoi / emit_name_first       = name_n))),
                name_n(                                       //
                    quot / emit_name_n_last  = expect_colon,  //
                    hsm::any / mem_start_str = name_n_cont,
                    name_n_ch(                                       //
                        hsm::any / mem_add_ch = hsm::internal,       //
                        name_n_cont(                                 //
                            quot / emit_name_n_last = expect_colon,  //
                            eoi / emit_name_n       = name_n)))),
            expect_colon(                                            //
                whitespace                         = expect_colon,   //
                colon                              = json_state,     //
                eoi                                = hsm::internal,  //
                hsm::any / error_action(colon_exp) = error),

            keyword(                                                                              //
                eoi                                                             = hsm::internal,  //
                hsm::any[kw_consume] / kw_consume_char                          = keyword,
                hsm::any[kw_complete] / kw_complete_keyword                     = array_object,  //
                hsm::any[kw_wrong_char] / error_action(wrong_keyword_character) = error          //
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
                array_object_br_close(                                                          //
                    hsm::initial[no_object_on_stack] / error_action(mismatched_brace) = error,  //
                    hsm::initial[object_on_stack] / pop_object                        = array_object),
                idx_close = array_object_idx_close,
                array_object_idx_close(                                                        //
                    hsm::initial[no_array_on_stack] / error_action(mismatched_array) = error,  //
                    hsm::initial[array_on_stack] / pop_array                         = array_object),
                hsm::any / error_action(comma_expected) = error  //
                ));
        sm.start();
        process_events = [this, sm = std::move(sm)](sv_t const& bytes, int ctrl) mutable {
            if (ctrl < 0) sm.start();
            current_input_buffer = bytes;
            auto switch_char     = [&sm](auto c) {
                switch (c)
                {
                    case 'n': return sm.process_event(n);
                    case 't': return sm.process_event(t);
                    case 'f': return sm.process_event(f);
                    case '{': return sm.process_event(br_open);
                    case '}': return sm.process_event(br_close);
                    case '[': return sm.process_event(idx_open);
                    case ']': return sm.process_event(idx_close);
                    case '"': return sm.process_event(quot);
                    case ':': return sm.process_event(colon);
                    case ',': return sm.process_event(comma);
                    case '+': return sm.process_event(plus);
                    case '-': return sm.process_event(minus);
                    case '0': /* fall-through */
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9': return sm.process_event(digit);
                    case 'E':
                    case 'e': return sm.process_event(exponent);
                    case '.': return sm.process_event(dot);
                    case ' ':
                    case '\t':
                    case '\r':
                    case '\n': return sm.process_event(whitespace);
                    default: return sm.process_event(ch);
                }
            };
            for (auto elem : bytes)
            {
                cur = elem;
                // TODO - validate utf8 characters
                switch_char(cur);
                if (sm.current_state_id() == sm.get_state_id(error)) return false;
                ++byte_count;
                current_input_buffer = sv_t(current_input_buffer.begin() + 1, current_input_buffer.size() - 1);
            }
            sm.process_event(eoi);
            return sm.current_state_id() != sm.get_state_id(error);
        };
    }

   public:
    explicit basic_json_parser(Handler&& handler) : cbs(std::move(handler)) { setup_sm(); }
    basic_json_parser() { setup_sm(); }

    Handler* callback_handler() { return &cbs; }
    bool     parse_bytes(sv_t const& input) { return process_events(input, 0); }
    void     reset()
    {
        state_stack.clear();
        num_sign    = 1;
        exp_sign    = 1;
        frac_digits = 0;
        exp_number  = 0;
        int_number  = 0;
        fraction    = 0;
        byte_count  = 0;
        process_events(sv_t{}, -1);
    }
};
}  // namespace async_json
#endif
