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

#include "dec/alice_soft/aff_file_decoder.h"
#include "algo/range.h"

// Doesn't encode anything, just wraps real files.

using namespace au;
using namespace au::dec::alice_soft;

static const bstr magic = "AFF\x00"_b;
static const bstr key =
    "\xC8\xBB\x8F\xB7\xED\x43\x99\x4A\xA2\x7E\x5B\xB0\x68\x18\xF8\x88"_b;

bool AffFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> AffFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size() + 4);
    const auto size = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);

    auto data = input_file.stream.read_to_eof();
    for (const auto i : algo::range(std::min<size_t>(data.size(), 64)))
        data[i] ^= key[i % key.size()];
    auto output_file = std::make_unique<io::File>(input_file.path, data);
    output_file->guess_extension();
    return output_file;
}

static auto _ = dec::register_decoder<AffFileDecoder>("alice-soft/aff");
