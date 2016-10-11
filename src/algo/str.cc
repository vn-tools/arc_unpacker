// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "algo/str.h"
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "algo/format.h"
#include "algo/range.h"

using namespace au;

std::string algo::lower(const std::string &input)
{
    return boost::algorithm::to_lower_copy(input);
}

bstr algo::reverse(const bstr &input)
{
    bstr output(input);
    std::reverse(output.begin(), output.end());
    return output;
}

std::string algo::reverse(const std::string &input)
{
    std::string output(input);
    std::reverse(output.begin(), output.end());
    return output;
}

std::string algo::trim_to_zero(const std::string &input)
{
    return std::string(input.c_str());
}

bstr algo::trim_to_zero(const bstr &input)
{
    return bstr(input.get<const char>());
}

std::string algo::hex(const bstr &input)
{
    std::string output;
    boost::algorithm::hex(
        input.begin(), input.end(), std::back_inserter(output));
    return output;
}

std::string algo::hex_verbose(const bstr &input, const size_t columns)
{
    if (!columns)
        return hex(input);
    std::string output;
    output.reserve(input.size() * 4.5);
    for (const auto y : range((input.size() + columns - 1) / columns))
    {
        output += format("%04x: ", y * columns);
        for (const auto x : range(columns))
        {
            const size_t i = x + y * columns;
            if (i < input.size())
                output += format("%02x ", input[i]);
            else
                output += "   ";
            if (i % 8 == 7)
                output += " ";
        }
        for (const auto x : range(columns))
        {
            const size_t i = x + y * columns;
            if (i >= input.size())
            {
                output += " ";
                continue;
            }
            output += format(
                "%c", input[i] >= 0x20 && input[i] < 0x7F ? input[i] : '.');
        }
        output += "\n";
    }
    return output;
}

bstr algo::unhex(const std::string &input)
{
    std::string output;
    boost::algorithm::unhex(input, std::back_inserter(output));
    return output;
}

std::vector<std::string> algo::split(
    const std::string &input,
    const char separator,
    const bool keep_separators)
{
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = input.find(separator, start)) != std::string::npos)
    {
        const auto temp = input.substr(
            start, end + (keep_separators ? 1 : 0) - start);
        if (!temp.empty())
            tokens.push_back(temp);
        start = end + 1;
    }
    const auto temp = input.substr(start);
    if (!temp.empty())
        tokens.push_back(temp);
    return tokens;
}

std::string algo::replace_all(
    const std::string &input,
    const std::string &from,
    const std::string &to)
{
    std::string output(input);
    boost::replace_all(output, from, to);
    return output;
}


namespace au {
namespace algo {

    template<> int from_string(const std::string &input)
    {
        return boost::lexical_cast<int>(input);
    }

    template<> float from_string(const std::string &input)
    {
        return boost::lexical_cast<float>(input);
    }

} }
