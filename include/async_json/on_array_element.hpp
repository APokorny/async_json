/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_ON_ARRAY_ELEMENT_HPP_INCLUDED
#define ASYNC_JSON_ON_ARRAY_ELEMENT_HPP_INCLUDED

#include <async_json/saj_event_mapper.hpp>
#include <hsm/hsm.hpp>
namespace async_json
{
namespace detail
{
constexpr hsm::state_ref<struct handle_values_s>   handle_values;
constexpr hsm::state_ref<struct count_objects_s>   count_objects;
constexpr hsm::state_ref<struct consume_strings_s> consume_strings;

struct on_array_element
{
    bool event_ret{false};
    bool event_has_value{false};
    int  depth_counter{0};
};

inline auto create_on_array_sm()
{
    auto is_value    = [](on_array_element& self) { return self.event_has_value; };
    auto is_zero     = [](on_array_element& self) { return self.depth_counter == 0; };
    auto ret_true    = [](on_array_element& self) { self.event_ret = true; };
    auto dec_counter = [](on_array_element& self) { --self.depth_counter; };
    auto inc_counter = [](on_array_element& self) { ++self.depth_counter; };

    auto sm = create_saj_state_machine<cj::on_array_element>(  //
        a::a_start = handle_values,
        handle_values(a::o_start                    = count_objects,    //
                      a::str_start                  = consume_strings,  //
                      hsm::any[is_value] / ret_true = handle_values),   //
        consume_strings(                                                //
            a::str_cont           = hsm::internal,
            a::str_end / ret_true = handle_values),        //
        a::a_end = hsm::root,                              //
        count_objects(                                     //
            a::o_end[is_zero] / ret_true = handle_values,  //
            a::o_end / dec_counter       = count_objects,  //
            a::o_start / inc_counter     = count_objects,  //
            a::a_end / dec_counter       = count_objects,  //
            a::a_start / inc_counter     = count_objects,  //
            hsm::any                     = count_objects)                      //
    );

    detail::on_array_element state{false, false, 0};
    sm.start(state);
    return tiny_tuple::tuple<detail::on_array_element, std::decay_t<decltype(sm)>>{state, sm};
}
}  // namespace detail

template <typename R>
inline constexpr auto on_array_element(R&& r) noexcept
{
    return [oa_sm = detail::create_on_array_sm(), r](auto const& ev) {
        auto& oa_state                 = tiny_tuple::get<0>(oa_sm);
        auto& sm                       = tiny_tuple::get<1>(oa_sm);
        on_array_state.event_has_value = ev.has_value();
        on_array_state.event_ret       = false;
        sm.process_event(static_cast<decltype(path_store)::event_id>(ev.as_event_id()), on_array_state);
        if (on_array_state.ret) r(ev);
    };
};

}  // namespace async_json

#endif
