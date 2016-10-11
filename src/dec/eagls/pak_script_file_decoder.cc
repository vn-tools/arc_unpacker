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

#include "dec/eagls/pak_script_file_decoder.h"
#include "algo/crypt/lcg.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::eagls;

static const bstr key = "EAGLS_SYSTEM"_b;

bool PakScriptFileDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("dat")
        || input_file.stream.size() < 3600)
    {
        return false;
    }

    for (const auto i : algo::range(100))
    {
        const auto name = input_file.stream.read(32).str();
        input_file.stream.skip(4);

        // after first zero, the "name" (whatever it is) should contain only
        // more zeros
        const auto zero_index = name.find_first_of('\x00');
        if (zero_index == std::string::npos)
            return false;
        for (const auto i : algo::range(zero_index, name.size()))
            if (name[i] != '\x00')
                return false;
    }

    return true;
}

std::unique_ptr<io::File> PakScriptFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    // According to Crass the offset, key and even the presence of LCG
    // vary for other games.

    const auto offset = 3600;
    input_file.stream.seek(offset);
    auto data = input_file.stream.read(input_file.stream.size() - offset - 1);
    const s8 seed = input_file.stream.read<u8>();
    algo::crypt::Lcg lcg(algo::crypt::LcgKind::MicrosoftVisualC, seed);
    for (const auto i : algo::range(0, data.size(), 2))
        data[i] ^= key[lcg.next() % key.size()];

    auto output_file = std::make_unique<io::File>(input_file.path, data);
    output_file->path.change_extension("txt");
    return output_file;
}

static auto _ = dec::register_decoder<PakScriptFileDecoder>("eagls/pak-script");
