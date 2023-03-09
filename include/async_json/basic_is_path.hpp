/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_IS_PATH_HPP_INCLUDED
#define ASYNC_JSON_IS_PATH_HPP_INCLUDED

#include <vector>
#include <functional>
#include <async_json/saj_event_mapper.hpp>
#include <hsm/hsm.hpp>

namespace async_json
{
struct unrolled_tag;
struct table_tag;

namespace detail
{
bool begins_with(std::string_view const& str, std::string_view const& b) noexcept;

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

}  // namespace detail
template <typename Traits, typename IT = table_tag>
struct basic_is_path
{
    using traits_type = Traits;
    using event_value = async_json::saj_event_value<Traits>;
    std::vector<detail::path_element> path_elements;
    explicit basic_is_path(std::initializer_list<detail::path_element> elements);
    basic_is_path(basic_is_path const&);
    basic_is_path(basic_is_path&&);
    basic_is_path& operator=(basic_is_path const&);
    basic_is_path& operator=(basic_is_path&&);
    bool           operator()(event_value const& ev);

    constexpr inline bool  at_end() const noexcept { return element == path_elements.size(); }
    constexpr inline int   depth() const noexcept { return dislocation; }
    bool matches_element_begin(std::string_view const& sv) noexcept;
    bool matches_element_part(std::string_view const& sv) noexcept;
    bool element_complete() noexcept;

   private:
    void setup_sm();

    std::function<bool(event_value const&)> path_handler;

    int         dislocation{0};
    size_t      element{0};
    size_t      pos{0};
    event_value cur;
};

constexpr detail::path_element arbitrary{detail::path_type::arbitrary};
#if defined(ASYNC_JSON_EXTERN) && !defined(ASYNC_JSON_EXPAND)
extern template class basic_is_path<default_traits, table_tag>;
#endif
}  // namespace async_json

#ifndef ASYNC_JSON_EXTERN
#include <async_json/detail/basic_is_path.ipp>
#endif
#endif
