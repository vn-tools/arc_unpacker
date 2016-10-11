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

#include "dec/borland/tpf0_decoder.h"
#include "algo/format.h"
#include "algo/locale.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::borland;

static const auto magic = "TPF0"_b;

static algo::any read_value(io::BaseByteStream &input_stream)
{
    const auto type = input_stream.read<u8>();
    if (type == 0)
        throw err::CorruptDataError("Zero not supported in this context");

    if (type == 2)
        return static_cast<int>(input_stream.read<u8>());

    if (type == 3)
        return static_cast<int>(input_stream.read_le<u16>());

    if (type == 5)
    {
        const auto hi = input_stream.read_le<u64>();
        const auto lo = input_stream.read_le<u16>();
        const u64 f80_sign     = lo >> 15;  // 1
        const u64 f80_exponent = lo & ~15;  // 15
        const u64 f80_mantissa = hi;        // 64
        const u64 f64_sign     = f80_sign;              // 1
        const u64 f64_exponent = f80_exponent >> 4;     // 11
        const u64 f64_mantissa = f80_mantissa >> 12;    // 52
        const u64 packed_f64
            = (f64_sign << 63)
            | (f64_exponent << 52)
            | (f64_mantissa << 0);
        const auto alias = reinterpret_cast<const char*>(&packed_f64);
        return *reinterpret_cast<const f64*>(alias);
    }

    if (type == 6 || type == 7)
        return input_stream.read(input_stream.read<u8>()).str();

    if (type == 8)
        return true;

    if (type == 9)
        return true;

    if (type == 10)
        return input_stream.read(input_stream.read_le<u32>());

    if (type == 11)
    {
        std::vector<std::string> ret;
        while (true)
        {
            const auto size = input_stream.read<u8>();
            if (size == 0)
                break;
            ret.push_back(input_stream.read(size).str());
        }
        return ret;
    }

    if (type == 18)
    {
        const auto size = input_stream.read_le<u32>();
        return algo::utf16_to_utf8(input_stream.read(size * 2)).str();
    }

    throw err::NotSupportedError(
        algo::format("Unknown value type: %d\n", type));
}

static std::unique_ptr<Tpf0Structure> read_structure(
    io::BaseByteStream &input_stream)
{
    const auto type_size = input_stream.read<u8>();
    auto s = std::make_unique<Tpf0Structure>();
    if (type_size == 0)
        return nullptr;
    s->type = input_stream.read(type_size).str();
    s->name = input_stream.read(input_stream.read<u8>()).str();
    while (true)
    {
        const auto key_size = input_stream.read<u8>();
        if (!key_size)
            break;
        const auto key = input_stream.read(key_size).str();
        s->properties[key] = read_value(input_stream);
    }
    while (true)
    {
        auto child = read_structure(input_stream);
        if (child == nullptr)
            break;
        s->children.push_back(std::move(child));
    }
    return s;
}

std::unique_ptr<Tpf0Structure> Tpf0Decoder::decode(const bstr &input) const
{
    io::MemoryByteStream input_stream(input);
    if (input_stream.read(magic.size()) != magic)
        throw err::RecognitionError();
    return read_structure(input_stream);
}
