/* ==========================================================================
 Copyright (c) 2022 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_DETAIL_BASIC_ON_ARRAY_ELEMENT_IPP_INCLUDED
#define ASYNC_JSON_DETAIL_BASIC_ON_ARRAY_ELEMENT_IPP_INCLUDED
#include <hsm/hsm.hpp>

namespace async_json
{
constexpr hsm::state_ref<struct handle_values_s>   handle_values;
constexpr hsm::state_ref<struct count_objects_s>   count_objects;
constexpr hsm::state_ref<struct consume_strings_s> consume_strings;

template <typename Traits, typename IT>
inline void basic_on_array_element<Traits, IT>::setup_sm()
{
    using self_t     = basic_on_array_element<Traits, IT>;
    auto is_value    = [](self_t& self) { return self.event_has_value; };
    auto is_zero     = [](self_t& self) { return self.depth_counter == 0; };
    auto ret_true    = [](self_t& self) { self.event_ret = true; };
    auto dec_counter = [](self_t& self) { --self.depth_counter; };
    auto inc_counter = [](self_t& self) { ++self.depth_counter; };

    auto sm = create_saj_state_machine<self_t, IT>(  //
        a_start = handle_values,
        handle_values(o_start                       = count_objects,    //
                      str_start                     = consume_strings,  //
                      hsm::any[is_value] / ret_true = handle_values),   //
        consume_strings(                                                //
            str_cont           = hsm::internal,
            str_end / ret_true = handle_values),        //
        a_end = hsm::root,                              //
        count_objects(                                  //
            o_end[is_zero] / ret_true = handle_values,  //
            o_end / dec_counter       = count_objects,  //
            o_start / inc_counter     = count_objects,  //
            a_end / dec_counter       = count_objects,  //
            a_start / inc_counter     = count_objects,  //
            hsm::any                  = count_objects)                   //
    );

    event_ret       = false;
    event_has_value = false;
    depth_counter   = 0;
    sm.start(*this);
    ev_handler = [sm = std::move(sm)](self_t* oa_state, event_value const& ev) mutable
    {
        oa_state->event_has_value = ev.has_value();
        oa_state->event_ret       = false;
        sm.process_event(static_cast<typename std::decay_t<decltype(sm)>::event_id>(ev.as_event_id()), *oa_state);
        return oa_state->event_ret;
    };
}

template <typename Traits, typename IT>
void basic_on_array_element<Traits, IT>::operator()(event_value const& ev)
{
    if (ev_handler(this, ev)) callback();
}

}  // namespace async_json

#endif
