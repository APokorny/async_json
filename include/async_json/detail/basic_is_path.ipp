/* ==========================================================================
 Copyright (c) 2022 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_BASIC_IS_PATH_IPP_INCLUDED
#define ASYNC_JSON_BASIC_IS_PATH_IPP_INCLUDED
namespace async_json
{
namespace detail
{
bool begins_with(std::string_view const& str, std::string_view const& b) noexcept
{
    auto i1 = str.begin();
    auto e1 = str.end();
    auto i2 = b.begin();
    auto e2 = b.end();
    for (; i1 != e1 && i2 != e2; ++i1, ++i2)
        if (*i1 != *i2) return false;
    return i2 == e2;
};

}  // namespace detail
constexpr hsm::state_ref<struct expect_object_s>         expect_object;
constexpr hsm::state_ref<struct expect_object_or_done_s> expect_object_or_done;
constexpr hsm::state_ref<struct in_path_s>               in_path;
constexpr hsm::state_ref<struct off_path_s>              off_path;
constexpr hsm::state_ref<struct off_path_in_object_s>    off_path_in_object;
constexpr hsm::state_ref<struct exp_value_s>             exp_value;
constexpr hsm::state_ref<struct value_or_empty_struct_s> value_or_empty_struct;
constexpr hsm::state_ref<struct object_value_nested_s>   object_value_nested;
constexpr hsm::state_ref<struct consume_array_s>         consume_array;
constexpr hsm::state_ref<struct consume_array_nested_s>  consume_array_nested;
constexpr hsm::state_ref<struct consume_value_s>         consume_value;
constexpr hsm::state_ref<struct ds_s>                    ds;

template <typename Traits>
basic_is_path<Traits>::basic_is_path(std::initializer_list<detail::path_element> elements) : path_elements(elements)
{
    setup_sm();
}
template <typename Traits>
basic_is_path<Traits>::basic_is_path(basic_is_path const& other) : path_elements{other.path_elements}
{
    setup_sm();
}

template <typename Traits>
basic_is_path<Traits>::basic_is_path(basic_is_path&& other) : path_elements{std::move(other.path_elements)}
{
    setup_sm();
}
template <typename Traits>
basic_is_path<Traits>& basic_is_path<Traits>::operator=(basic_is_path const& other)
{
    path_elements = other.path_elements;
    element       = 0;
    dislocation   = 0;
    pos           = 0;
    return *this;
}
template <typename Traits>
basic_is_path<Traits>& basic_is_path<Traits>::operator=(basic_is_path&& other)
{
    path_elements = std::move(other.path_elements);
    element       = 0;
    dislocation   = 0;
    pos           = 0;
    return *this;
}

template <typename Traits>
bool basic_is_path<Traits>::operator()(event_value const& ev)
{
    cur = ev;
    return path_handler(ev);
}

template <typename Traits>
bool basic_is_path<Traits>::matches_element_begin(std::string_view const& sv) noexcept
{
    if (at_end()) return false;
    auto& pe = path_elements[element];
    if (pe.type == detail::path_type::arbitrary || detail::begins_with(pe.str.substr(0), sv))
    {
        pos += sv.size();
        return true;
    }
    return false;
}
template <typename Traits>
bool basic_is_path<Traits>::matches_element_part(std::string_view const& sv) noexcept
{
    if (at_end()) return false;
    auto& pe = path_elements[element];
    if (pos != 0 && (pe.type == detail::path_type::arbitrary || detail::begins_with(pe.str.substr(pos), sv)))
    {
        pos += sv.size();
        return true;
    }
    pos = 0;
    return false;
}

template <typename Traits>
bool basic_is_path<Traits>::element_complete() noexcept
{
    if (at_end()) return false;
    auto& pe = path_elements[element];
    if (pe.type != detail::path_type::arbitrary && pos != pe.str.size())
    {
        pos = 0;
        return false;
    }
    pos         = 0;
    pe.in_array = false;
    ++element;

    return true;
}

template <typename Traits>
void basic_is_path<Traits>::setup_sm()
{
    using self_t                   = basic_is_path<Traits>;
    auto match_begin               = [](self_t& self) { return self.matches_element_begin(self.cur.as_string_view()); };
    auto match_part                = [](self_t& self) { return self.matches_element_part(self.cur.as_string_view()); };
    auto match_end                 = [](self_t& self) { return self.element_complete(); };
    auto last_element              = [](self_t& self) { return self.at_end(); };
    auto off_path_counter_not_zero = [](self_t& self) { return self.dislocation >= 1; };
    auto inc_off_path_counter      = [](self_t& self) { ++self.dislocation; };
    auto dec_off_path_counter      = [](self_t& self)
    {
        if (self.dislocation) --self.dislocation;
    };
    auto remove_element = [](self_t& self)
    {
        if (self.element) --self.element;
    };
    auto is_value  = [](self_t& self) { return self.cur.has_value(); };
    auto put_array = [](self_t& self)
    {
        if (self.element) self.path_elements[self.element - 1].in_array = true;
    };
    auto is_in_array = [](self_t& self)
    {
        if (self.element)
            return self.path_elements[self.element - 1].in_array;
        else
            return false;
    };

    namespace a  = async_json;
    auto path_sm = a::create_saj_state_machine<self_t>(           //
        hsm::initial[last_element] = exp_value,                   //
        a::o_start                 = in_path,                     //
        expect_object(                                            //
            a::o_start                          = in_path,        //
            a::on_start[match_begin]            = in_path,        //
            a::a_start / put_array              = expect_object,  // change
            a::str_start                        = consume_value,  //
            hsm::any[is_value] / remove_element = in_path),       //
        expect_object_or_done,                                    //
        expect_object_or_done[last_element] = exp_value,          //
        expect_object_or_done               = expect_object,      //
        consume_value(                                            //
            a::str_cont = consume_value, a::str_end / remove_element = in_path),
        consume_array(                                                                       //
            a::o_start                = consume_array_nested,                                //
            a::a_start                = consume_array_nested,                                //
            a::a_end / remove_element = in_path,                                             //
            hsm::any                  = consume_array,                                       //
            consume_array_nested(                                                            //
                a::o_start / inc_off_path_counter                          = hsm::internal,  //
                a::a_start / inc_off_path_counter                          = hsm::internal,  //
                a::o_end[off_path_counter_not_zero] / dec_off_path_counter = hsm::internal,  //
                a::a_end[off_path_counter_not_zero] / dec_off_path_counter = hsm::internal,  //
                a::o_end                                                   = consume_array,  //
                a::a_end                                                   = consume_array,  //
                hsm::any                                                   = consume_array_nested)),
        in_path(                                                //
            a::o_end[is_in_array]     = expect_object,          // added
            a::o_end / remove_element = in_path,                //
            a::on_start[match_begin]  = hsm::internal,          //
            a::on_cont[match_part]    = hsm::internal,          //
            a::on_end[match_end]      = expect_object_or_done,  //
            a::on_start               = off_path,               //
            a::on_cont                = off_path,               //
            a::on_end                 = off_path),                              //
        off_path(                                               //
            a::o_start                = off_path_in_object,     //
            a::o_end / remove_element = in_path,                //
            a::on_start[match_begin]  = in_path,                //
            hsm::any                  = off_path,
            off_path_in_object(                                                              //
                a::o_start / inc_off_path_counter                          = hsm::internal,  //
                a::o_end[off_path_counter_not_zero] / dec_off_path_counter = hsm::internal,  //
                a::o_end                                                   = off_path,       //
                hsm::any                                                   = off_path_in_object)),                                             //
        exp_value(                                                                           //
            a::o_start                = value_or_empty_struct,                               //
            a::a_start                = value_or_empty_struct,                               //
            a::str_start              = exp_value,                                           //
            a::str_cont               = exp_value,                                           //
            hsm::any / remove_element = in_path,                                             //
            value_or_empty_struct(                                                           //
                a::o_start / inc_off_path_counter   = value_or_empty_struct,                 //
                a::a_start / inc_off_path_counter   = value_or_empty_struct,                 //
                a::o_end[off_path_counter_not_zero] = object_value_nested,                   //
                a::a_end[off_path_counter_not_zero] = object_value_nested,                   //
                a::o_end / remove_element           = in_path,                               //
                a::a_end / remove_element           = in_path,                               //
                hsm::any / inc_off_path_counter     = object_value_nested,                   //
                object_value_nested(                                                         //
                    a::o_start = value_or_empty_struct,                                      //
                    a::a_start = value_or_empty_struct,                                      //
                    ds(hsm::initial[off_path_counter_not_zero] = object_value_nested,        //
                       hsm::initial / remove_element           = in_path),                             //
                    a::o_end / dec_off_path_counter = ds,                                    //
                    a::a_end / dec_off_path_counter = ds,                                    //
                    hsm::any                        = object_value_nested)                                          //
                )));

    path_sm.start(*this);
    path_handler = [path_sm = std::move(path_sm), this](event_value const& ev) mutable
    {
        bool ret = path_sm.current_state_id() == path_sm.get_state_id(exp_value) ||
                   path_sm.current_state_id() == path_sm.get_state_id(value_or_empty_struct) ||
                   path_sm.current_state_id() == path_sm.get_state_id(object_value_nested);
        path_sm.process_event(static_cast<typename decltype(path_sm)::event_id>(ev.as_event_id()), *this);
        return ret;
    };
}
}  // namespace async_json

#endif
