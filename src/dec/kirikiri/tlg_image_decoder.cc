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

#include "dec/kirikiri/tlg_image_decoder.h"
#include "dec/kirikiri/tlg/tlg5_decoder.h"
#include "dec/kirikiri/tlg/tlg6_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::kirikiri;
using namespace au::dec::kirikiri::tlg;

static const bstr magic_tlg_0 = "TLG0.0\x00sds\x1A"_b;
static const bstr magic_tlg_5 = "TLG5.0\x00raw\x1A"_b;
static const bstr magic_tlg_6 = "TLG6.0\x00raw\x1A"_b;

static int guess_version(io::BaseByteStream &input_stream);
static res::Image decode_proxy(int version, io::File &input_file);

static std::string extract_string(std::string &container)
{
    size_t size = 0;
    while (container.size() && container[0] >= '0' && container[0] <= '9')
    {
        size *= 10;
        size += container[0] - '0';
        container.erase(0, 1);
    }
    container.erase(0, 1);
    std::string str = container.substr(0, size);
    container.erase(0, size + 1);
    return str;
}

static res::Image decode_tlg_0(io::File &input_file)
{
    const auto raw_data_size = input_file.stream.read_le<u32>();
    const auto raw_data_offset = input_file.stream.pos();

    std::vector<std::pair<std::string, std::string>> tags;

    input_file.stream.skip(raw_data_size);
    while (input_file.stream.left())
    {
        const auto chunk_name = input_file.stream.read(4).str();
        const auto chunk_size = input_file.stream.read_le<u32>();
        auto chunk_data = input_file.stream.read(chunk_size).str();

        if (chunk_name == "tags")
        {
            while (chunk_data != "")
            {
                const auto key = extract_string(chunk_data);
                const auto value = extract_string(chunk_data);
                tags.push_back(std::pair<std::string, std::string>(key, value));
            }
        }
        else
            throw err::NotSupportedError("Unknown chunk: " + chunk_name);
    }

    input_file.stream.seek(raw_data_offset);
    int version = guess_version(input_file.stream);
    if (version == -1)
        throw err::UnsupportedVersionError();
    return decode_proxy(version, input_file);
}

static res::Image decode_tlg_5(io::File &input_file)
{
    return Tlg5Decoder().decode(input_file);
}

static res::Image decode_tlg_6(io::File &input_file)
{
    return Tlg6Decoder().decode(input_file);
}

static int guess_version(io::BaseByteStream &input_stream)
{
    const auto pos = input_stream.pos();
    if (input_stream.read(magic_tlg_0.size()) == magic_tlg_0)
        return 0;

    input_stream.seek(pos);
    if (input_stream.read(magic_tlg_5.size()) == magic_tlg_5)
        return 5;

    input_stream.seek(pos);
    if (input_stream.read(magic_tlg_6.size()) == magic_tlg_6)
        return 6;

    return -1;
}

static res::Image decode_proxy(int version, io::File &input_file)
{
    switch (version)
    {
        case 0:
            return decode_tlg_0(input_file);

        case 5:
            return decode_tlg_5(input_file);

        case 6:
            return decode_tlg_6(input_file);
    }
    throw std::logic_error("Unknown TLG version");
}

bool TlgImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return guess_version(input_file.stream) >= 0;
}

res::Image TlgImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    int version = guess_version(input_file.stream);
    return decode_proxy(version, input_file);
}

static auto _ = dec::register_decoder<TlgImageDecoder>("kirikiri/tlg");
