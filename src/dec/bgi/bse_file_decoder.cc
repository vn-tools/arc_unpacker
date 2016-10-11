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

#include "dec/bgi/bse_file_decoder.h"
#include "algo/binary.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::bgi;

static const bstr magic = "BSE 1.0\x00"_b;

namespace
{
    class BseDecoder final
    {
    public:
        BseDecoder(io::File &input_file);
        bstr decode();

        const u32 data_size = 64;

    private:
        s32 rand();
        void check_sums() const;

        bstr data;
        s32 seed;
        u8 check_sum;
        u8 check_xor;
    };
}

BseDecoder::BseDecoder(io::File &input_file)
{
    input_file.stream.skip(magic.size());
    input_file.stream.skip(2);
    check_sum = input_file.stream.read<u8>();
    check_xor = input_file.stream.read<u8>();
    seed = input_file.stream.read_le<s32>();
    data = input_file.stream.read(data_size);
}

s32 BseDecoder::rand()
{
    const auto tmp = (((seed * 257 >> 8) + seed * 97) + 23) ^ 0xA6CD9B75;
    seed = algo::rotr(tmp, 16);
    return seed & 0x7FFF;
}

void BseDecoder::check_sums() const
{
    u8 data_sum = 0;
    u8 data_xor = 0;
    for (const auto i : algo::range(0, data_size))
    {
        data_sum += data[i];
        data_xor ^= data[i];
    }
    if (data_sum != check_sum || data_xor != check_xor)
        throw err::CorruptDataError("Wrong check sum");
}

bstr BseDecoder::decode()
{
    std::vector<bool>flags(data_size, false);
    for (const auto _ : algo::range(data_size))
    {
        auto i = rand() % data_size;
        while (flags[i])
            i = (i + 1) % data_size;

        const auto shift = rand() % 8;
        const auto key   = rand();
        const auto data  = this->data[i] - rand();

        if (key & 1)
            this->data[i] = algo::rotl<u8>(data, shift);
        else
            this->data[i] = algo::rotr<u8>(data, shift);

        flags[i] = true;
    }
    check_sums();
    return data;
}

bool BseFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> BseFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    BseDecoder decoder(input_file);
    const auto data = decoder.decode();

    auto output_file = std::make_unique<io::File>();
    output_file->stream.write(data);
    output_file->stream.write(input_file.stream.read_to_eof());
    output_file->path = input_file.path;
    if (!output_file->path.has_extension())
        output_file->path.change_extension("dat");
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> BseFileDecoder::get_linked_formats() const
{
    return {"bgi/cbg", "bgi/dsc"};
}

static auto _ = dec::register_decoder<BseFileDecoder>("bgi/bse");
