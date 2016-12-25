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

#include "dec/kaguya/common/params_encryption.h"
#include "algo/binary.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::kaguya;

static bool compare_sjis(const bstr &input1, const bstr &input2)
{
    return algo::normalize_sjis(input1) == algo::normalize_sjis(input2);
}

static bstr read_binary_string(io::BaseByteStream &input_stream)
{
    return input_stream.read(input_stream.read<u8>());
}

static bstr read_sjis_string(io::BaseByteStream &input_stream)
{
    return algo::sjis_to_utf8(input_stream.read(input_stream.read<u8>()));
}

static bstr read_utf16_string(io::BaseByteStream &input_stream)
{
    return algo::utf16_to_utf8(input_stream.read(input_stream.read_le<u16>()));
}

static bool verify_magic(
    io::BaseByteStream &input_stream,
    const std::initializer_list<bstr> &magic_list)
{
    for (const auto &magic : magic_list)
        if (input_stream.seek(0).read(magic.size()) == magic)
            return true;
    return false;
}

static bool verify_magic(io::BaseByteStream &input_stream, const bstr &magic)
{
    return verify_magic(input_stream, {magic});
}

static void decrypt(
    io::BaseByteStream &input_stream, const bstr &key, const size_t size)
{
    const auto old_pos = input_stream.pos();
    const auto data = algo::unxor(input_stream.read(size), key);
    input_stream.seek(old_pos);
    input_stream.write(data);
}

static void decrypt(io::BaseByteStream &input_stream, const bstr &key)
{
    decrypt(input_stream, key, input_stream.left());
}

static int get_scr_version(io::BaseByteStream &input_stream)
{
    if (verify_magic(input_stream, "[SCR-PARAMS]v05.3"_b)) return 53;
    if (verify_magic(input_stream, "[SCR-PARAMS]v05.2"_b)) return 52;
    if (verify_magic(input_stream, "[SCR-PARAMS]v05.1"_b)) return 51;
    if (verify_magic(input_stream, "[SCR-PARAMS]v05"_b)) return 50;
    if (verify_magic(input_stream, "[SCR-PARAMS]v04"_b)) return 40;
    if (verify_magic(input_stream, "[SCR-PARAMS]v03"_b)) return 30;
    if (verify_magic(input_stream, "[SCR-PARAMS]v02"_b)) return 20;
    if (verify_magic(input_stream, "[SCR-PARAMS]v01"_b)) return 10;
    throw err::RecognitionError();
}

static common::Params parse_params_file_v1(io::BaseByteStream &input_stream)
{
    input_stream.skip(8);
    input_stream.skip(4 * input_stream.read_le<u32>());
    const auto game_title = algo::sjis_to_utf8(input_stream.read_to_zero());
    input_stream.read_to_zero();
    const auto producer = algo::sjis_to_utf8(input_stream.read_to_zero());
    const auto copyright = algo::sjis_to_utf8(input_stream.read_to_zero());
    for (const auto i : algo::range(2))
    {
        input_stream.skip(4);
        input_stream.read_to_zero();
    }
    for (const auto i : algo::range(input_stream.read_le<u32>()))
    {
        input_stream.read_to_zero();
        input_stream.read_to_zero();
    }

    for (const auto i : algo::range(input_stream.read_le<u32>()))
    {
        input_stream.skip(4);
        input_stream.skip(1);
        input_stream.skip(input_stream.read_le<u32>());
        for (const auto j : algo::range(input_stream.read_le<u32>()))
            input_stream.read_to_zero();
        input_stream.skip(1);
        input_stream.skip(input_stream.read_le<u32>());
    }

    for (const auto i : algo::range(input_stream.read_le<u32>()))
        input_stream.read_to_zero();

    for (const auto i : algo::range(input_stream.read_le<u32>()))
        input_stream.read_to_zero();

    const auto key_size = std::min<size_t>(240000, input_stream.read_le<u32>());
    common::Params params;
    params.decrypt_anm = false;
    params.game_title = game_title;
    params.key = input_stream.read(key_size);
    return params;
}

static common::Params parse_params_file_v2(io::BaseByteStream &input_stream)
{
    input_stream.skip(8);
    read_binary_string(input_stream);
    const auto game_title = read_sjis_string(input_stream);
    const auto producer = read_sjis_string(input_stream);
    const auto copyright = read_sjis_string(input_stream);
    read_binary_string(input_stream);
    input_stream.skip(1);
    for (const auto i : algo::range(2))
        read_binary_string(input_stream);
    for (const auto i : algo::range(input_stream.read<u8>()))
    {
        const auto arc_name = input_stream.read(input_stream.read<u8>());
        const auto arc_type = input_stream.read(input_stream.read<u8>());
    }
    input_stream.skip(1);

    size_t key_size;

    if (compare_sjis(game_title, "幼なじみと甘～くエッチに過ごす方法"_b)
        || compare_sjis(game_title, "艶女医"_b))
    {
        for (const auto i : algo::range(input_stream.read<u8>()))
        {
            input_stream.skip(1);
            read_binary_string(input_stream);
            for (const auto j : algo::range(input_stream.read<u8>()))
                read_binary_string(input_stream);
            read_binary_string(input_stream);
        }

        for (const auto i : algo::range(input_stream.read<u8>()))
            read_binary_string(input_stream);

        for (const auto i : algo::range(input_stream.read<u8>()))
            read_binary_string(input_stream);

        key_size = input_stream.read_le<u32>();
        if (compare_sjis(game_title, "幼なじみと甘～くエッチに過ごす方法"_b))
            key_size = 240000;
    }

    else if (compare_sjis(game_title, "新妻イカせてミルク！"_b)
        || compare_sjis(game_title, "毎日がＭ！"_b)
        || compare_sjis(game_title, "ちゅぱしてあげる"_b))
    {
        for (const auto i : algo::range(input_stream.read<u8>()))
        {
            input_stream.skip(1);
            read_binary_string(input_stream);
            for (const auto j : algo::range(2))
            for (const auto k : algo::range(input_stream.read<u8>()))
                read_binary_string(input_stream);
        }

        for (const auto i : algo::range(input_stream.read<u8>()))
        {
            read_binary_string(input_stream);
            read_binary_string(input_stream);
        }

        for (const auto i : algo::range(input_stream.read<u8>()))
        {
            read_binary_string(input_stream);
            for (const auto j : algo::range(2))
            for (const auto k : algo::range(input_stream.read<u8>()))
                read_binary_string(input_stream);
        }

        key_size = input_stream.read_le<u32>();
    }
    else
    {
        throw err::CorruptDataError("Unknown game: " + game_title.str());
    }

    common::Params params;
    params.decrypt_anm = game_title != "幼なじみと甘～くエッチに過ごす方法"_b;
    params.game_title = game_title;
    params.key = input_stream.read(key_size);
    return params;
}

static void skip_tree(io::BaseByteStream &input_stream)
{
    read_utf16_string(input_stream);
    for (const auto i : algo::range(input_stream.read_le<u32>()))
    {
        read_utf16_string(input_stream);
        read_utf16_string(input_stream);
    }
    for (const auto i : algo::range(input_stream.read_le<u32>()))
        skip_tree(input_stream);
}

static common::Params parse_params_file_v3_or_later(
    io::BaseByteStream &input_stream, const size_t version)
{
    const auto read_string = version < 50
        ? read_sjis_string
        : read_utf16_string;
    // v3 and v4 use some idiotic encoding which we're going to ignore
    const auto read_string_safe = version < 50
        ? read_binary_string
        : read_utf16_string;

    input_stream.skip(10);
    read_binary_string(input_stream);
    const auto game_title = read_string(input_stream);
    const auto producer = read_string(input_stream);
    const auto copyright = read_string(input_stream);

    input_stream.skip(1);
    read_string_safe(input_stream);
    read_string_safe(input_stream);

    for (const auto i : algo::range(input_stream.read<u8>()))
    {
        read_string_safe(input_stream);
        read_string_safe(input_stream);
    }

    input_stream.skip(12);

    if (version < 52)
    {
        if (version < 50)
            input_stream.skip(1);

        for (const auto i : algo::range(input_stream.read<u8>()))
        {
            input_stream.skip(1);
            read_string_safe(input_stream);
            for (const auto j : algo::range(2))
            for (const auto k : algo::range(input_stream.read<u8>()))
                read_string_safe(input_stream);
        }

        for (const auto i : algo::range(input_stream.read<u8>()))
        {
            read_string_safe(input_stream);
            read_string_safe(input_stream);
        }

        for (const auto i : algo::range(input_stream.read<u8>()))
        {
            read_string_safe(input_stream);
            for (const auto j : algo::range(2))
            for (const auto j : algo::range(input_stream.read<u8>()))
                read_string_safe(input_stream);
        }
    }
    else
    {
        input_stream.skip(4);
        for (const auto i : algo::range(3))
            if (input_stream.read<u8>())
                skip_tree(input_stream);

        if (version >= 53)
            input_stream.skip(input_stream.read_le<u32>() * 12);
    }

    const auto key_size = input_stream.read_le<u32>();
    common::Params params;
    params.decrypt_anm = true;
    params.game_title = game_title;
    params.key = input_stream.read(key_size);
    return params;
}

common::Params common::parse_params_file(io::BaseByteStream &input_stream)
{
    const auto version = get_scr_version(input_stream);
    if (version < 20) return parse_params_file_v1(input_stream);
    if (version < 30) return parse_params_file_v2(input_stream);
    return parse_params_file_v3_or_later(input_stream, version);
}

void common::decrypt(
    io::BaseByteStream &input_stream, const common::Params &params)
{
    if (verify_magic(input_stream, "BM"_b))
        ::decrypt(input_stream.seek(54), params.key);

    else if (verify_magic(input_stream, {"AP-3"_b, "AP-2"_b}))
         ::decrypt(input_stream.seek(24), params.key);

    else if (verify_magic(input_stream, {"AP-1"_b, "AP-0"_b, "AP"_b}))
         ::decrypt(input_stream.seek(12), params.key);

    else if (verify_magic(input_stream, "AN00"_b) && params.decrypt_anm)
    {
        input_stream.seek(20);
        const auto frame_count = input_stream.read_le<u16>();
        input_stream.skip(2 + frame_count * 4);
        const auto file_count = input_stream.read_le<u16>();
        for (const auto i : algo::range(file_count))
        {
            input_stream.skip(8);
            const auto width = input_stream.read_le<u32>();
            const auto height = input_stream.read_le<u32>();
            ::decrypt(input_stream, params.key, 4 * width * height);
        }
    }

    else if (verify_magic(input_stream, "AN10"_b) && params.decrypt_anm)
    {
        input_stream.seek(20);
        const auto frame_count = input_stream.read_le<u16>();
        input_stream.skip(2 + frame_count * 4);
        const auto file_count = input_stream.read_le<u16>();
        for (const auto i : algo::range(file_count))
        {
            input_stream.skip(8);
            const auto width = input_stream.read_le<u32>();
            const auto height = input_stream.read_le<u32>();
            const auto channels = input_stream.read_le<u32>();
            ::decrypt(input_stream, params.key, channels * width * height);
        }
    }

    else if (verify_magic(input_stream, "AN20"_b) && params.decrypt_anm)
    {
        input_stream.seek(4);
        const auto unk_count = input_stream.read_le<u16>();
        input_stream.skip(2);
        for (const auto i : algo::range(unk_count))
        {
            const auto control = input_stream.read<u8>();
            if (control == 0) continue;
            else if (control == 1) input_stream.skip(8);
            else if (control == 2) input_stream.skip(4);
            else if (control == 3) input_stream.skip(4);
            else if (control == 4) input_stream.skip(4);
            else if (control == 5) input_stream.skip(4);
            else throw err::NotSupportedError("Unsupported control");
        }
        const auto unk2_count = input_stream.read_le<u16>();
        input_stream.skip(unk2_count * 8);
        const auto file_count = input_stream.read_le<u16>();
        if (!file_count)
            return;
        input_stream.skip(16);
        for (const auto i : algo::range(file_count))
        {
            input_stream.skip(8);
            const auto width = input_stream.read_le<u32>();
            const auto height = input_stream.read_le<u32>();
            const auto channels = input_stream.read_le<u32>();
            ::decrypt(input_stream, params.key, channels * width * height);
        }
    }

    else if (verify_magic(input_stream, "AN21"_b) && params.decrypt_anm)
    {
        input_stream.seek(4);
        const auto unk_count = input_stream.read_le<u16>();
        input_stream.skip(2);
        for (const auto i : algo::range(unk_count))
        {
            const auto control = input_stream.read<u8>();
            if (control == 0) continue;
            else if (control == 1) input_stream.skip(8);
            else if (control == 2) input_stream.skip(4);
            else if (control == 3) input_stream.skip(4);
            else if (control == 4) input_stream.skip(4);
            else if (control == 5) input_stream.skip(4);
            else throw err::NotSupportedError("Unsupported control");
        }
        const auto unk2_count = input_stream.read_le<u16>();
        input_stream.skip(unk2_count * 8);
        input_stream.skip(7);
        const auto file_count = input_stream.read_le<u16>();
        if (!file_count)
            return;
        input_stream.skip(24);
        const auto width = input_stream.read_le<u32>();
        const auto height = input_stream.read_le<u32>();
        const auto channels = input_stream.read_le<u32>();
        ::decrypt(input_stream, params.key, channels * width * height);
    }

    else if (verify_magic(input_stream, "PL00"_b))
    {
        input_stream.seek(4);
        const auto file_count = input_stream.read_le<u16>();
        input_stream.skip(16);
        for (const auto i : algo::range(file_count))
        {
            input_stream.skip(8);
            const auto width = input_stream.read_le<u32>();
            const auto height = input_stream.read_le<u32>();
            const auto channels = input_stream.read_le<u32>();
            ::decrypt(input_stream, params.key, channels * width * height);
        }
    }

    else if (verify_magic(input_stream, "PL10"_b))
    {
        input_stream.seek(4);
        const auto file_count = input_stream.read_le<u16>();
        input_stream.skip(16);
        input_stream.skip(8);
        const auto width = input_stream.read_le<u32>();
        const auto height = input_stream.read_le<u32>();
        const auto channels = input_stream.read_le<u32>();
        ::decrypt(input_stream, params.key, channels * width * height);
    }
}
