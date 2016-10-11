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

#include "dec/wild_bug/wbi_file_decoder.h"
#include "algo/range.h"
#include "dec/wild_bug/wpx/decoder.h"

using namespace au;
using namespace au::dec::wild_bug;

static const bstr magic = "WPX\x1A""EX2\x00"_b;

bool WbiFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> WbiFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    wpx::Decoder decoder(input_file.stream);
    const auto data = decoder.read_compressed_section(0x02);
    auto output_file = std::make_unique<io::File>(input_file.path, data);
    output_file->path.change_extension("dat");
    return output_file;
}

static auto _ = dec::register_decoder<WbiFileDecoder>("wild-bug/wbi");
