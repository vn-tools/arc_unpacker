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

#include "dec/fc01/mca_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "algo/str.h"
#include "dec/fc01/common/custom_lzss.h"
#include "dec/fc01/common/util.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::fc01;

static const bstr magic = "MCA "_b;

namespace
{
    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        uoff_t offset;
    };
}

static bstr decrypt(const bstr &input, size_t output_size, u8 initial_key)
{
    bstr output(input.size());
    auto key = initial_key;
    for (const auto i : algo::range(input.size()))
    {
        output[i] = common::rol8(input[i], 1) ^ key;
        key += input.size() - i;
    }
    return output;
}

McaArchiveDecoder::McaArchiveDecoder()
{
    add_arg_parser_decorator(
        [](ArgParser &arg_parser)
        {
            arg_parser.register_switch({"--mca-key"})
                ->set_value_name("KEY")
                ->set_description(
                    "Decryption key (0..255, same for all files)");
        },
        [&](const ArgParser &arg_parser)
        {
            if (arg_parser.has_switch("mca-key"))
                key = algo::from_string<int>(arg_parser.get_switch("mca-key"));
        });
}

bool McaArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> McaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(16);
    const auto header_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(12);
    const auto file_count = input_file.stream.read_le<u32>();
    input_file.stream.seek(header_size);

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = algo::format("%03d.png", i);
        entry->offset = input_file.stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> McaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);
    const auto encryption_type = input_file.stream.read_le<u32>();
    input_file.stream.skip(8);
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    const auto size_comp = input_file.stream.read_le<u32>();
    const auto size_orig = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);

    auto data = input_file.stream.read(size_comp);

    if (!key)
        throw err::UsageError("MCA decryption key not set");
    if (encryption_type == 0)
    {
        data = decrypt(data, size_orig, key.get());
    }
    else if (encryption_type == 1)
    {
        data = decrypt(data, size_orig, key.get());
        data = common::custom_lzss_decompress(data, size_orig);
    }
    else
    {
        throw err::NotSupportedError(algo::format(
            "Unknown encryption type: %d", encryption_type));
    }

    data = common::fix_stride(data, width, height, 24);
    res::Image image(width, height, data, res::PixelFormat::BGR888);
    const auto encoder = enc::png::PngImageEncoder();
    return encoder.encode(logger, image, entry->path);
}

static auto _ = dec::register_decoder<McaArchiveDecoder>("fc01/mca");
