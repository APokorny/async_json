/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_IS_PATH_HPP_INCLUDED
#define ASYNC_JSON_IS_PATH_HPP_INCLUDED

#include <async_json/saj_event_mapper.hpp>
#include <hsm/hsm.hpp>

namespace async_json
{
namespace detail
{
bool begins_with(std::string_view const& str, std::string_view const& b)
{
    auto i1 = str.begin();
    auto e1 = str.end();
    auto i2 = b.begin();
    auto e2 = b.end();
    for (; i1 != e1 && i2 != e2; ++i1, ++i2)
        if (*i1 != *i2) return false;
    return i2 == e2;
}

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

enum class path_type : uint8_t
{
    string,
    arbitrary
};

struct path_element
{
    std::string_view str;
    path_type        type;
    bool             in_array{false};
    constexpr path_element(std::string_view const& s) : str{s}, type{path_type::string} {}
    constexpr path_element(char const* s) : str{s}, type{path_type::string} {}
    constexpr path_element(path_type pt) : str{}, type{pt} {}
};

struct is_path
{
    std::vector<path_element> path_elements;
    template <typename... Ts>
    is_path(Ts&&... ts) : path_elements{ts...}
    {
    }
    bool at_end() const { return element == path_elements.size(); }
    int  depth() const { return dislocation; }
    bool matches_element_begin(std::string_view const& sv)
    {
        if (at_end()) return false;
        auto& pe = path_elements[element];
        if (pe.type == path_type::arbitrary || begins_with(pe.str.substr(0), sv))
        {
            pos += sv.size();
            return true;
        }
        return false;
    }

    bool matches_element_part(std::string_view const& sv)
    {
        if (at_end()) return false;
        auto& pe = path_elements[element];
        if (pos != 0 && (pe.type == path_type::arbitrary || begins_with(pe.str.substr(pos), sv)))
        {
            pos += sv.size();
            return true;
        }
        pos = 0;
        return false;
    }

    bool element_complete()
    {
        if (at_end()) return false;
        auto& pe = path_elements[element];
        if (pe.type != path_type::arbitrary && pos != pe.str.size())
        {
            pos = 0;
            return false;
        }
        pos         = 0;
        pe.in_array = false;
        ++element;

        return true;
    }
    int              dislocation{0};
    size_t           element{0};
    size_t           pos{0};
    bool             has_value{false};
    std::string_view str_val{};
};

inline auto create_path_sm(is_path&& obj)
{
    auto match_begin               = [](is_path& self) { return self.matches_element_begin(self.str_val); };
    auto match_part                = [](is_path& self) { return self.matches_element_part(self.str_val); };
    auto match_end                 = [](is_path& self) { return self.element_complete(); };
    auto last_element              = [](is_path& self) { return self.at_end(); };
    auto off_path_counter_not_zero = [](is_path& self) { return self.dislocation >= 1; };
    auto inc_off_path_counter      = [](is_path& self) { ++self.dislocation; };
    auto dec_off_path_counter      = [](is_path& self) {
        if (self.dislocation) --self.dislocation;
    };
    auto remove_element = [](is_path& self) {
        if (self.element) --self.element;
    };
    auto is_value  = [](is_path& self) { return self.has_value; };
    auto put_array = [](is_path& self) {
        if (self.element) self.path_elements[self.element - 1].in_array = true;
    };
    auto is_in_array = [](is_path& self) { return (self.element) ? self.path_elements[self.element - 1].in_array : false; };

    auto sm = create_saj_state_machine<is_path>(                  //
        hsm::initial[last_element] = exp_value,                   //
        o_start                    = in_path,                     //
        expect_object(                                            //
            o_start                             = in_path,        //
            on_start[match_begin]               = in_path,        //
            a_start / put_array                 = expect_object,  // change
            str_start                           = consume_value,  //
            hsm::any[is_value] / remove_element = in_path),       //
        expect_object_or_done,                                    //
        expect_object_or_done[last_element] = exp_value,          //
        expect_object_or_done               = expect_object,      //
        consume_value(                                            //
            str_cont = consume_value, str_end / remove_element = in_path),
        consume_array(                                                                    //
            o_start                = consume_array_nested,                                //
            a_start                = consume_array_nested,                                //
            a_end / remove_element = in_path,                                             //
            hsm::any               = consume_array,                                       //
            consume_array_nested(                                                         //
                o_start / inc_off_path_counter                          = hsm::internal,  //
                a_start / inc_off_path_counter                          = hsm::internal,  //
                o_end[off_path_counter_not_zero] / dec_off_path_counter = hsm::internal,  //
                a_end[off_path_counter_not_zero] / dec_off_path_counter = hsm::internal,  //
                o_end                                                   = consume_array,  //
                a_end                                                   = consume_array,  //
                hsm::any                                                = consume_array_nested)),
        in_path(                                             //
            o_end[is_in_array]     = expect_object,          // added
            o_end / remove_element = in_path,                //
            on_start[match_begin]  = hsm::internal,          //
            on_cont[match_part]    = hsm::internal,          //
            on_end[match_end]      = expect_object_or_done,  //
            on_start               = off_path,               //
            on_cont                = off_path,               //
            on_end                 = off_path),                              //
        off_path(                                            //
            o_start                = off_path_in_object,     //
            o_end / remove_element = in_path,                //
            on_start[match_begin]  = in_path,                //
            hsm::any               = off_path,
            off_path_in_object(                                                           //
                o_start / inc_off_path_counter                          = hsm::internal,  //
                o_end[off_path_counter_not_zero] / dec_off_path_counter = hsm::internal,  //
                o_end                                                   = off_path,       //
                hsm::any                                                = off_path_in_object)),                                          //
        exp_value(                                                                        //
            o_start                   = value_or_empty_struct,                            //
            a_start                   = value_or_empty_struct,                            //
            str_start                 = exp_value,                                        //
            str_cont                  = exp_value,                                        //
            hsm::any / remove_element = in_path,                                          //
            value_or_empty_struct(                                                        //
                o_start / inc_off_path_counter   = value_or_empty_struct,                 //
                a_start / inc_off_path_counter   = value_or_empty_struct,                 //
                o_end[off_path_counter_not_zero] = object_value_nested,                   //
                a_end[off_path_counter_not_zero] = object_value_nested,                   //
                o_end / remove_element           = in_path,                               //
                a_end / remove_element           = in_path,                               //
                hsm::any / inc_off_path_counter  = object_value_nested,                   //
                object_value_nested(                                                      //
                    o_start = value_or_empty_struct,                                      //
                    a_start = value_or_empty_struct,                                      //
                    ds(hsm::initial[off_path_counter_not_zero] = object_value_nested,     //
                       hsm::initial / remove_element           = in_path),                          //
                    o_end / dec_off_path_counter = ds,                                    //
                    a_end / dec_off_path_counter = ds,                                    //
                    hsm::any                     = object_value_nested)                                       //
                )));
    sm.start(obj);
    return tiny_tuple::tuple<std::decay_t<decltype(obj)>, std::decay_t<decltype(sm)>>{obj, sm};
}
}  // namespace detail

template <typename... Ts>
inline auto is_path(Ts&&... ts) noexcept
{
    return [path_sm = detail::create_path_sm(detail::is_path(std::forward<Ts>(ts)...))](auto const& ev) mutable {
        auto& path_state     = tiny_tuple::get<0>(path_sm);
        auto& sm             = tiny_tuple::get<1>(path_sm);
        path_state.has_value = ev.has_value();
        path_state.str_val   = (ev.event == saj_event::object_name_cont || ev.event == saj_event::object_name_start) ? ev.as_string_view()
                                                                                                                   : std::string_view{};

        bool ret = sm.current_state_id() == sm.get_state_id(detail::exp_value) ||
                   sm.current_state_id() == sm.get_state_id(detail::value_or_empty_struct) ||
                   sm.current_state_id() == sm.get_state_id(detail::object_value_nested);
        sm.process_event(static_cast<std::decay_t<decltype(sm)>::event_id>(ev.as_event_id()), path_state);
        return ret;
    };
};
constexpr detail::path_element arbitrary{detail::path_type::arbitrary};

}  // namespace async_json

#endif
