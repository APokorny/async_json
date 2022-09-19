/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef ASYNC_JSON_STRING_CONVERSION_HPP_INCLUDED
#define ASYNC_JSON_STRING_CONVERSION_HPP_INCLUDED
#include <string>
namespace async_json
{
namespace detail
{
inline bool         is_hex(char t) noexcept { return (t >= '0' && t <= '9') || (t >= 'a' && t <= 'f') || (t >= 'A' && t <= 'F'); }
inline unsigned int from_hex(char t) noexcept
{
    return static_cast<unsigned int>(t - ((t >= '0' && t <= '9') ? '0' : (t >= 'a' && t <= 'f') ? 'a' : 'A'));
}
}  // namespace detail
inline std::string& json_to_utf8(std::string& str)
{
    enum escape_state
    {
        none,
        bs,
        uhex1,
        uhex2,
        uhex3,
        uhex4,
        hex_done
    };
    escape_state       es{none};
    unsigned int const factors[4]        = {12u, 8u, 4u, 0u};
    int                codepoint         = 0;
    auto*              factor            = factors;
    auto               consume_codepoint = [&str, &factor, &es](int& cp, size_t i) -> bool {
        if (detail::is_hex(str[i]))
        {
            cp += static_cast<int>(detail::from_hex(str[i]) << *factor++);
            es = static_cast<decltype(es)>(static_cast<int>(es) + 1);
            return true;
        }
        else
        {
            return false;
        }
    };
    for (size_t i = 0; i != str.size(); ++i)
    {
        switch (es)
        {
            case none:
                if (str[i] == '\\') es = bs;
                break;
            case bs:
            {
                char const* rep = nullptr;
                switch (str[i])
                {
                    case '\\': rep = "\\"; break;
                    case 'b': rep = "\b"; break;
                    case 't': rep = "\t"; break;
                    case 'r': rep = "\r"; break;
                    case 'n': rep = "\n"; break;
                    case '"': rep = "\""; break;
                    case 'u':
                        if (str.size() - i < 4) return str;  // with error
                        es        = uhex1;
                        factor    = factors;
                        codepoint = 0;
                        continue;
                    default: return str;  // with error;
                }
                if (rep)
                {
                    str.replace(--i, 2, rep);
                    --i;
                    es = none;
                }
                break;
            }
            case uhex1:
            case uhex2:
            case uhex3:
            case uhex4:
                if (!consume_codepoint(codepoint, i)) return str;  // error
                if (es == hex_done)
                {
                    size_t chars_to_replace = 6;
                    // two consecutive codepoints:
                    if (0xD800 <= codepoint && codepoint <= 0xDBFF)
                    {
                        chars_to_replace += 6;
                        int cp2 = 0;
                        es      = uhex1;
                        factor  = factors;
                        if (i + 6 < str.size() && str[i + 1] == '\\' && str[i + 2] == 'u' && consume_codepoint(cp2, i + 3) &&
                            consume_codepoint(cp2, i + 4) && consume_codepoint(cp2, i + 5) && consume_codepoint(cp2, i + 6))
                        {
                            if (0xDC00 <= cp2 && cp2 <= 0xDFFF)
                                // overwrite codepoint
                                codepoint = static_cast<int>(
                                    // high surrogate occupies the most significant 22 bits
                                    (static_cast<unsigned int>(codepoint) << 10u)
                                    // low surrogate occupies the least significant 15 bits
                                    + static_cast<unsigned int>(cp2)
                                    // there is still the 0xD800, 0xDC00 and 0x10000 noise
                                    // in the result so we have to subtract with:
                                    // (0xD800 << 10) + DC00 - 0x10000 = 0x35FDC00
                                    - 0x35FDC00u);
                            else
                                return str;  // error
                        }
                        else
                        {
                            return str;  // error
                        }
                    }
                    else if (0xDC00 <= codepoint && codepoint <= 0xDFFF)
                        return str;  // error
                    char array[5]        = {0, 0, 0, 0, 0};
                    auto replacement_pos = i - 5;
                    if (codepoint < 0x80)
                    {
                        array[0] = static_cast<char>(codepoint);
                        str.replace(replacement_pos, chars_to_replace, array);
                        i -= 6;
                    }
                    else if (codepoint <= 0x7FF)
                    {
                        array[0] = static_cast<char>(0xC0u | (static_cast<unsigned int>(codepoint) >> 6u));
                        array[1] = static_cast<char>(0x80u | (static_cast<unsigned int>(codepoint) & 0x3Fu));
                        str.replace(replacement_pos, chars_to_replace, array);
                        i -= 5;
                    }
                    else if (codepoint <= 0xFFFF)
                    {
                        // 3-byte characters: 1110xxxx 10xxxxxx 10xxxxxx
                        array[0] = static_cast<char>(0xE0u | (static_cast<unsigned int>(codepoint) >> 12u));
                        array[1] = static_cast<char>(0x80u | ((static_cast<unsigned int>(codepoint) >> 6u) & 0x3Fu));
                        array[2] = static_cast<char>(0x80u | (static_cast<unsigned int>(codepoint) & 0x3Fu));
                        str.replace(replacement_pos, chars_to_replace, array);
                        i -= 4;
                    }
                    else
                    {
                        // 4-byte characters: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                        array[0] = static_cast<char>(0xF0u | (static_cast<unsigned int>(codepoint) >> 18u));
                        array[1] = static_cast<char>(0x80u | ((static_cast<unsigned int>(codepoint) >> 12u) & 0x3Fu));
                        array[2] = static_cast<char>(0x80u | ((static_cast<unsigned int>(codepoint) >> 6u) & 0x3Fu));
                        array[3] = static_cast<char>(0x80u | (static_cast<unsigned int>(codepoint) & 0x3Fu));
                        str.replace(replacement_pos, chars_to_replace, array);
                        i -= 3;
                    }
                    es        = none;
                }
                break;
            case hex_done:
            default:
                break;
        }
    }
    return str;
}

inline std::string json_to_utf8(std::string const& str)
{
    std::string ret{str};
    json_to_utf8(ret);
    return ret;
}
inline std::string json_to_utf8(std::string&& str)
{
    json_to_utf8(str);
    return std::move(str);
}
}  // namespace async_json

#endif
