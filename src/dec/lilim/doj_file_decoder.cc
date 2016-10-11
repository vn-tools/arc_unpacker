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

#include "dec/lilim/doj_file_decoder.h"
#include "dec/lilim/common.h"
#include "err.h"

using namespace au;
using namespace au::dec::lilim;

static const bstr magic1 = "CC"_b;
static const bstr magic2 = "DD"_b;

bool DojFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("doj")
        && input_file.stream.read(magic1.size()) == magic1;
}

std::unique_ptr<io::File> DojFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic1.size());
    const auto meta_size = input_file.stream.read_le<u16>() * 6;
    input_file.stream.skip(meta_size);
    if (input_file.stream.read(magic2.size()) != magic2)
        throw err::CorruptDataError("Corrupt metadata");

    input_file.stream.skip(2);
    const auto size_comp = input_file.stream.read_le<u32>();
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto data = sysd_decompress(input_file.stream.read(size_comp));
    if (data.size() != size_orig)
        throw err::BadDataSizeError();
    return std::make_unique<io::File>(input_file.path, data);
}

static auto _ = dec::register_decoder<DojFileDecoder>("lilim/doj");
