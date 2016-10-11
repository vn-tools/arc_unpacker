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

#include "dec/alice_soft/ajp_image_decoder.h"
#include "algo/range.h"
#include "dec/alice_soft/pms_image_decoder.h"
#include "dec/jpeg/jpeg_image_decoder.h"

using namespace au;
using namespace au::dec::alice_soft;

static const bstr magic = "AJP\x00"_b;
static const bstr key =
    "\x5D\x91\xAE\x87\x4A\x56\x41\xCD\x83\xEC\x4C\x92\xB5\xCB\x16\x34"_b;

static void decrypt(bstr &input)
{
    for (const auto i : algo::range(std::min(input.size(), key.size())))
        input[i] ^= key[i];
}

bool AjpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image AjpImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    input_file.stream.skip(4 * 2);
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    const auto jpeg_offset = input_file.stream.read_le<u32>();
    const auto jpeg_size = input_file.stream.read_le<u32>();
    const auto mask_offset = input_file.stream.read_le<u32>();
    const auto mask_size = input_file.stream.read_le<u32>();

    input_file.stream.seek(jpeg_offset);
    auto jpeg_data = input_file.stream.read(jpeg_size);
    decrypt(jpeg_data);

    input_file.stream.seek(mask_offset);
    auto mask_data = input_file.stream.read(mask_size);
    decrypt(mask_data);

    const auto jpeg_image_decoder = dec::jpeg::JpegImageDecoder();
    io::File jpeg_file;
    jpeg_file.stream.write(jpeg_data);
    auto image = jpeg_image_decoder.decode(logger, jpeg_file);

    if (mask_size)
    {
        const auto pms_image_decoder = PmsImageDecoder();
        io::File mask_file;
        mask_file.stream.write(mask_data);
        const auto mask_image = pms_image_decoder.decode(logger, mask_file);
        image.apply_mask(mask_image);
    }

    return image;
}

static auto _ = dec::register_decoder<AjpImageDecoder>("alice-soft/ajp");
