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

#include "algo/locale.h"
#include <cerrno>
#include <iconv.h>
#include <memory>
#include "err.h"

using namespace au;

static bstr replace(const bstr &input, const bstr &from, const bstr &to)
{
    bstr output(input);
    size_t index = 0;
    while (true)
    {
        index = output.find(from, index);
        if (index == bstr::npos)
            break;
        output.replace(index, from.size(), to);
        index += to.size();
    }
    return output;
}

static bstr convert_locale(
    const bstr &input, const std::string &from, const std::string &to)
{
    iconv_t conv = iconv_open(to.c_str(), from.c_str());
    if (!conv)
        throw std::logic_error("Failed to initialize iconv");

    bstr output;
    output.reserve(input.size() * 2);

    auto input_ptr = const_cast<char*>(input.get<const char>());
    auto input_bytes_left = input.size();
    bstr buffer(32);

    while (true)
    {
        auto output_buffer = buffer.get<char>();
        auto output_bytes_left = buffer.size();
        int ret = iconv(
            conv,
            &input_ptr,
            &input_bytes_left,
            &output_buffer,
            &output_bytes_left);
        const auto err = errno;

        output += buffer.substr(0, buffer.size() - output_bytes_left);

        if (ret != -1 && input_bytes_left == 0)
            break;

        // repeat the iteration unless nothing was parsed at all
        if (err == E2BIG && output_bytes_left != buffer.size())
            continue;

        iconv_close(conv);
        if (err == E2BIG)
            throw err::CorruptDataError("Code point too large to decode (?)");
        else if (err == EINVAL || err == EILSEQ)
            throw err::CorruptDataError("Invalid byte sequence");
        else
            throw err::CorruptDataError("Unknown iconv error");
    }

    iconv_close(conv);
    return output;
}

bstr algo::sjis_to_utf8(const bstr &input)
{
    return convert_locale(input, "cp932", "utf-8");
}

bstr algo::utf16_to_utf8(const bstr &input)
{
    return convert_locale(input, "utf-16le", "utf-8");
}

bstr algo::utf8_to_sjis(const bstr &input)
{
    return convert_locale(input, "utf-8", "cp932");
}

bstr algo::utf8_to_utf16(const bstr &input)
{
    return convert_locale(input, "utf-8", "utf-16le");
}

bstr algo::normalize_sjis(const bstr &utf8_input)
{
    // WAVE DASH to FULLWIDTH TILDE
    return replace(utf8_input, "〜"_b, "～"_b);
}
