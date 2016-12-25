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
#include <array>
#include <stack>
#include "algo/locale.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"

using namespace au;
using namespace au::dec::kaguya::common;

static void write_raw(io::BaseByteStream &stream, const bstr &str)
{
    stream.write<u8>(str.size());
    stream.write(str);
}

static void write_sjis(io::BaseByteStream &stream, const bstr &str)
{
    write_raw(stream, algo::utf8_to_sjis(str));
}

static void write_utf16(io::BaseByteStream &stream, const bstr &str)
{
    stream.write_le<u16>(algo::utf8_to_utf16(str).size());
    stream.write(algo::utf8_to_utf16(str));
}

static void write_body_v1(
    io::BaseByteStream &output_stream, const bstr &key, const bstr &game_title)
{
    output_stream.write_le<u32>('?');
    output_stream.write_le<u32>('?');
    const auto unk0 = {1, 2, 3};
    output_stream.write_le<u32>(unk0.size());
    for (const auto b : unk0)
        output_stream.write_le<u32>(b);
    output_stream.write(algo::utf8_to_sjis(game_title));
    output_stream.write<u8>(0);
    write_raw(output_stream, "???"_b);
    output_stream.write<u8>(0);
    output_stream.write("producer"_b);
    output_stream.write<u8>(0);
    output_stream.write("copyright"_b);
    output_stream.write<u8>(0);
    for (const auto i : algo::range(2))
    {
        output_stream.write_le<u32>('?');
        output_stream.write("???"_b);
        output_stream.write<u8>(0);
    }

    const std::vector<std::pair<bstr, bstr>> arc_names
        = {{"bla"_b, "bla"_b}, {"herp"_b, "derp"_b}};
    output_stream.write_le<u32>(arc_names.size());
    for (const auto &kv : arc_names)
    {
        output_stream.write(kv.first);
        output_stream.write<u8>(0);
        output_stream.write(kv.second);
        output_stream.write<u8>(0);
    }

    const std::vector<std::vector<bstr>> unk1 = {
        {"12"_b, "3"_b, "45"_b, "67"_b, "89"_b},
        {"123"_b, "45"_b, "67"_b, "89"_b}};
    const std::vector<bstr> unk2 = {"12"_b, "3"_b, "45"_b};
    const std::vector<bstr> unk3 = {"12"_b, "3"_b, "45"_b, "67"_b, "89"_b};
    output_stream.write_le<u32>(unk1.size());
    for (const auto &t : unk1)
    {
        output_stream.write_le<u32>('?');
        output_stream.write<u8>('?');
        output_stream.write_le<u32>(t.at(0).size());
        output_stream.write(t.at(0));
        output_stream.write_le<u32>(t.size());
        for (const auto &s : t)
        {
            output_stream.write(s);
            output_stream.write<u8>(0);
        }
        output_stream.write<u8>('?');
        output_stream.write_le<u32>(t.at(0).size());
        output_stream.write(t.at(0));
    }
    output_stream.write_le<u32>(unk2.size());
    for (const auto &s : unk2)
    {
        output_stream.write(s);
        output_stream.write<u8>(0);
    }
    output_stream.write_le<u32>(unk3.size());
    for (const auto &s : unk3)
    {
        output_stream.write(s);
        output_stream.write<u8>(0);
    }
    output_stream.write_le<u32>(key.size());
    output_stream.write(key);
}

static void write_body_v2(
    io::BaseByteStream &output_stream, const bstr &key, const bstr &game_title)
{
    output_stream.write_le<u32>('?');
    output_stream.write_le<u32>('?');
    write_raw(output_stream, "???"_b);
    write_sjis(output_stream, game_title);
    write_sjis(output_stream, "producer"_b);
    write_sjis(output_stream, "copyright"_b);
    write_sjis(output_stream, "???"_b);
    output_stream.write<u8>('?');
    write_sjis(output_stream, "???"_b);
    write_sjis(output_stream, "???"_b);

    const std::vector<std::pair<bstr, bstr>> arc_names
        = {{"bla"_b, "bla"_b}, {"herp"_b, "derp"_b}};
    output_stream.write<u8>(arc_names.size());
    for (const auto &kv : arc_names)
    {
        write_sjis(output_stream, kv.first);
        write_sjis(output_stream, kv.second);
    }
    output_stream.write<u8>('?');

    const std::vector<std::vector<bstr>> unk = {
        {"12"_b, "3"_b, "45"_b, "67"_b, "89"_b},
        {"123"_b, "45"_b, "67"_b, "89"_b}};
    const std::vector<std::pair<bstr, bstr>> unk2 = {
        {"12"_b, "67"_b}, {"3"_b, "89"_b}, {"45"_b, "0"_b}};
    const std::vector<std::vector<bstr>> unk3 = {
        {"12"_b, "3"_b, "45"_b, "67"_b, "89"_b},
        {"123"_b, "45"_b, "67"_b, "89"_b}};
    output_stream.write<u8>(unk.size());
    for (const auto &t : unk)
    {
        output_stream.write<u8>('?');
        write_sjis(output_stream, "???"_b);
        output_stream.write<u8>(t.size());
        for (const auto &tt : t)
            write_sjis(output_stream, tt);
        output_stream.write<u8>(t.size());
        for (const auto &tt : t)
            write_sjis(output_stream, tt);
    }
    output_stream.write<u8>(unk2.size());
    for (const auto &s : unk2)
    {
        write_sjis(output_stream, s.first);
        write_sjis(output_stream, s.second);
    }
    output_stream.write<u8>(unk3.size());
    for (const auto &t : unk3)
    {
        write_sjis(output_stream, "???"_b);
        output_stream.write<u8>(t.size());
        for (const auto &tt : t)
            write_sjis(output_stream, tt);
        output_stream.write<u8>(t.size());
        for (const auto &tt : t)
            write_sjis(output_stream, tt);
    }
    output_stream.write_le<u32>(key.size());
    output_stream.write(key);
}

static void write_body_v3(
    io::BaseByteStream &output_stream, const bstr &key, const bstr &game_title)
{
    output_stream.write_le<u16>('?');
    output_stream.write_le<u32>('?');
    output_stream.write_le<u32>('?');
    write_raw(output_stream, "???"_b);
    write_sjis(output_stream, game_title);
    write_sjis(output_stream, "producer"_b);
    write_sjis(output_stream, "copyright"_b);
    output_stream.write<u8>('?');
    write_sjis(output_stream, "???"_b);
    write_sjis(output_stream, "???"_b);

    const std::vector<std::pair<bstr, bstr>> arc_names
        = {{"bla"_b, "bla"_b}, {"herp"_b, "derp"_b}};
    output_stream.write<u8>(arc_names.size());
    for (const auto &kv : arc_names)
    {
        write_sjis(output_stream, kv.first);
        write_sjis(output_stream, kv.second);
    }
    output_stream.write_le<u32>(3);
    output_stream.write_le<u32>(2);
    output_stream.write_le<u32>(1);
    output_stream.write<u8>('?');

    const std::vector<std::vector<bstr>> unk = {
        {"12"_b, "3"_b, "45"_b, "67"_b, "89"_b},
        {"123"_b, "45"_b, "67"_b, "89"_b}};
    const std::vector<std::pair<bstr, bstr>> unk2 = {
        {"12"_b, "67"_b}, {"3"_b, "89"_b}, {"45"_b, "0"_b}};
    const std::vector<std::vector<bstr>> unk3 = {
        {"12"_b, "3"_b, "45"_b, "67"_b, "89"_b},
        {"123"_b, "45"_b, "67"_b, "89"_b}};
    output_stream.write<u8>(unk.size());
    for (const auto &t : unk)
    {
        output_stream.write<u8>('?');
        write_sjis(output_stream, "???"_b);
        output_stream.write<u8>(t.size());
        for (const auto &tt : t)
            write_sjis(output_stream, tt);
        output_stream.write<u8>(t.size());
        for (const auto &tt : t)
            write_sjis(output_stream, tt);
    }
    output_stream.write<u8>(unk2.size());
    for (const auto &s : unk2)
    {
        write_sjis(output_stream, s.first);
        write_sjis(output_stream, s.second);
    }
    output_stream.write<u8>(unk3.size());
    for (const auto &t : unk3)
    {
        write_sjis(output_stream, "???"_b);
        output_stream.write<u8>(t.size());
        for (const auto &tt : t)
            write_sjis(output_stream, tt);
        output_stream.write<u8>(t.size());
        for (const auto &tt : t)
            write_sjis(output_stream, tt);
    }
    output_stream.write_le<u32>(key.size());
    output_stream.write(key);
}

static void write_body_v5(
    io::BaseByteStream &output_stream, const bstr &key, const bstr &game_title)
{
    output_stream.write_le<u16>('?');
    output_stream.write_le<u32>('?');
    output_stream.write_le<u32>('?');
    write_raw(output_stream, "???"_b);
    write_utf16(output_stream, game_title);
    write_utf16(output_stream, "producer"_b);
    write_utf16(output_stream, "copyright"_b);
    output_stream.write<u8>('?');
    write_utf16(output_stream, "???"_b);
    write_utf16(output_stream, "???"_b);

    const std::vector<std::pair<bstr, bstr>> arc_names
        = {{"bla"_b, "bla"_b}, {"herp"_b, "derp"_b}};
    output_stream.write<u8>(arc_names.size());
    for (const auto &kv : arc_names)
    {
        write_utf16(output_stream, kv.first);
        write_utf16(output_stream, kv.second);
    }
    output_stream.write_le<u32>(3);
    output_stream.write_le<u32>(2);
    output_stream.write_le<u32>(1);

    const std::vector<std::vector<bstr>> unk = {
        {"12"_b, "3"_b, "45"_b, "67"_b, "89"_b},
        {"123"_b, "45"_b, "67"_b, "89"_b}};
    const std::vector<std::pair<bstr, bstr>> unk2 = {
        {"12"_b, "67"_b}, {"3"_b, "89"_b}, {"45"_b, "0"_b}};
    const std::vector<std::vector<bstr>> unk3 = {
        {"12"_b, "3"_b, "45"_b, "67"_b, "89"_b},
        {"123"_b, "45"_b, "67"_b, "89"_b}};
    output_stream.write<u8>(unk.size());
    for (const auto &t : unk)
    {
        output_stream.write<u8>('?');
        write_utf16(output_stream, "???"_b);
        output_stream.write<u8>(t.size());
        for (const auto &tt : t)
            write_utf16(output_stream, tt);
        output_stream.write<u8>(t.size());
        for (const auto &tt : t)
            write_utf16(output_stream, tt);
    }
    output_stream.write<u8>(unk2.size());
    for (const auto &s : unk2)
    {
        write_utf16(output_stream, s.first);
        write_utf16(output_stream, s.second);
    }
    output_stream.write<u8>(unk3.size());
    for (const auto &t : unk3)
    {
        write_utf16(output_stream, "???"_b);
        output_stream.write<u8>(t.size());
        for (const auto &tt : t)
            write_utf16(output_stream, tt);
        output_stream.write<u8>(t.size());
        for (const auto &tt : t)
            write_utf16(output_stream, tt);
    }
    output_stream.write_le<u32>(key.size());
    output_stream.write(key);
}

static void write_body_v5_2(
    io::BaseByteStream &output_stream, const bstr &key, const bstr &game_title)
{
    output_stream.write_le<u16>('?');
    output_stream.write_le<u32>('?');
    output_stream.write_le<u32>('?');
    write_raw(output_stream, "???"_b);
    write_utf16(output_stream, game_title);
    write_utf16(output_stream, "producer"_b);
    write_utf16(output_stream, "copyright"_b);
    output_stream.write<u8>('?');
    write_utf16(output_stream, "???"_b);
    write_utf16(output_stream, "???"_b);

    const std::vector<std::pair<bstr, bstr>> arc_names
        = {{"bla"_b, "bla"_b}, {"herp"_b, "derp"_b}};
    output_stream.write<u8>(arc_names.size());
    for (const auto &kv : arc_names)
    {
        write_utf16(output_stream, kv.first);
        write_utf16(output_stream, kv.second);
    }
    output_stream.write_le<u32>(3);
    output_stream.write_le<u32>(2);
    output_stream.write_le<u32>(1);
    output_stream.write_le<u32>('?');

    struct Tree final
    {
        bstr name;
        std::vector<std::pair<bstr, bstr>> values;
        std::vector<Tree> children;
    };

    for (const auto i : algo::range(3))
    {
        Tree unk = {
            "whatever"_b,
            {{"1"_b, "2"_b}, {"3"_b, "4"_b}},
            {
                {
                    "nested1"_b,
                    {{"a"_b, "b"_b}, {"c"_b, "d"_b}, {"e"_b, "f"_b}},
                    {}
                },
                {
                    "nested2"_b,
                    {{"a"_b, "b"_b}, {"c"_b, "d"_b}, {"e"_b, "f"_b}},
                    {}
                },
            }
        };

        output_stream.write<u8>(1);

        std::stack<Tree> stack;
        stack.push(unk);
        while (!stack.empty())
        {
            const auto tree = stack.top();
            stack.pop();
            write_utf16(output_stream, tree.name);
            output_stream.write_le<u32>(tree.values.size());
            for (const auto &value : tree.values)
            {
                write_utf16(output_stream, value.first);
                write_utf16(output_stream, value.second);
            }
            output_stream.write_le<u32>(tree.children.size());
            for (const auto child : tree.children)
                stack.push(child);
        }
    }

    output_stream.write_le<u32>(key.size());
    output_stream.write(key);
}

static void write_body_v5_3(
    io::BaseByteStream &output_stream, const bstr &key, const bstr &game_title)
{
    output_stream.write_le<u16>('?');
    output_stream.write_le<u32>('?');
    output_stream.write_le<u32>('?');
    write_raw(output_stream, "???"_b);
    write_utf16(output_stream, game_title);
    write_utf16(output_stream, "producer"_b);
    write_utf16(output_stream, "copyright"_b);
    output_stream.write<u8>('?');
    write_utf16(output_stream, "???"_b);
    write_utf16(output_stream, "???"_b);

    const std::vector<std::pair<bstr, bstr>> arc_names
        = {{"bla"_b, "bla"_b}, {"herp"_b, "derp"_b}};
    output_stream.write<u8>(arc_names.size());
    for (const auto &kv : arc_names)
    {
        write_utf16(output_stream, kv.first);
        write_utf16(output_stream, kv.second);
    }
    output_stream.write_le<u32>(3);
    output_stream.write_le<u32>(2);
    output_stream.write_le<u32>(1);
    output_stream.write_le<u32>('?');

    struct Tree final
    {
        bstr name;
        std::vector<std::pair<bstr, bstr>> values;
        std::vector<Tree> children;
    };

    for (const auto i : algo::range(3))
    {
        Tree unk = {
            "whatever"_b,
            {{"1"_b, "2"_b}, {"3"_b, "4"_b}},
            {
                {
                    "nested1"_b,
                    {{"a"_b, "b"_b}, {"c"_b, "d"_b}, {"e"_b, "f"_b}},
                    {}
                },
                {
                    "nested2"_b,
                    {{"a"_b, "b"_b}, {"c"_b, "d"_b}, {"e"_b, "f"_b}},
                    {}
                },
            }
        };

        output_stream.write<u8>(1);

        std::stack<Tree> stack;
        stack.push(unk);
        while (!stack.empty())
        {
            const auto tree = stack.top();
            stack.pop();
            write_utf16(output_stream, tree.name);
            output_stream.write_le<u32>(tree.values.size());
            for (const auto &value : tree.values)
            {
                write_utf16(output_stream, value.first);
                write_utf16(output_stream, value.second);
            }
            output_stream.write_le<u32>(tree.children.size());
            for (const auto child : tree.children)
                stack.push(child);
        }
    }

    const std::vector<std::array<u32, 3>> unk = {{1, 2, 3}, {4, 5, 6}};
    output_stream.write_le<u32>(unk.size());
    for (const auto &t : unk)
        for (const auto tt : t)
            output_stream.write_le<u32>(tt);

    output_stream.write_le<u32>(key.size());
    output_stream.write(key);
}

TEST_CASE("Atelier Kaguya params decryption", "[dec]")
{
    // for unknown data, serialize nonempty junk to test skipping

    SECTION("Version 1")
    {
        const auto key = bstr(240000, 0xFF);
        const auto game_title = "毎日がＭ！"_b;
        io::MemoryByteStream output_stream;

        output_stream.write("[SCR-PARAMS]v01"_b);
        write_body_v1(output_stream, key, game_title);

        const auto params = parse_params_file(output_stream);
        REQUIRE(params.key == key);
        REQUIRE(params.game_title == game_title);
    }

    SECTION("Version 2 - variant A")
    {
        const auto key = bstr(240000, 0xFF);
        const auto game_title = "幼なじみと甘～くエッチに過ごす方法"_b;
        io::MemoryByteStream output_stream;

        output_stream.write("[SCR-PARAMS]v02"_b);
        output_stream.write_le<u32>('?');
        output_stream.write_le<u32>('?');
        write_raw(output_stream, "???"_b);
        write_sjis(output_stream, game_title);
        write_sjis(output_stream, "producer"_b);
        write_sjis(output_stream, "copyright"_b);
        write_sjis(output_stream, "???"_b);
        output_stream.write<u8>('?');
        write_sjis(output_stream, "???"_b);
        write_sjis(output_stream, "???"_b);

        const std::vector<std::pair<bstr, bstr>> arc_names
            = {{"bla"_b, "bla"_b}, {"herp"_b, "derp"_b}};
        output_stream.write<u8>(arc_names.size());
        for (const auto &kv : arc_names)
        {
            write_sjis(output_stream, kv.first);
            write_sjis(output_stream, kv.second);
        }
        output_stream.write<u8>('?');

        const std::vector<std::vector<bstr>> unk = {
            {"12"_b, "3"_b, "45"_b, "67"_b, "89"_b},
            {"123"_b, "45"_b, "67"_b, "89"_b}};
        const std::vector<bstr> unk2 = {"12"_b, "3"_b, "45"_b};
        const std::vector<bstr> unk3 = {"123"_b, "45"_b};
        output_stream.write<u8>(unk.size());
        for (const auto &t : unk)
        {
            output_stream.write<u8>('?');
            write_sjis(output_stream, "???"_b);
            output_stream.write<u8>(t.size());
            for (const auto &tt : t)
                write_sjis(output_stream, tt);
            write_sjis(output_stream, "???"_b);
        }
        output_stream.write<u8>(unk2.size());
        for (const auto &s : unk2)
            write_sjis(output_stream, s);
        output_stream.write<u8>(unk3.size());
        for (const auto &s : unk3)
            write_sjis(output_stream, s);
        output_stream.write_le<u32>(key.size());
        output_stream.write(key);

        const auto params = parse_params_file(output_stream);
        REQUIRE(params.key == key);
        REQUIRE(
            algo::normalize_sjis(params.game_title)
            == algo::normalize_sjis(game_title));
    }

    SECTION("Version 2 - variant B")
    {
        const auto key = "abc"_b;
        const auto game_title = "毎日がＭ！"_b;
        io::MemoryByteStream output_stream;

        output_stream.write("[SCR-PARAMS]v02"_b);
        write_body_v2(output_stream, key, game_title);

        const auto params = parse_params_file(output_stream);
        REQUIRE(params.key == key);
        REQUIRE(params.game_title == game_title);
    }

    SECTION("Version 3")
    {
        const auto key = "abc"_b;
        const auto game_title = "some generic title"_b;
        io::MemoryByteStream output_stream;
        output_stream.write("[SCR-PARAMS]v03"_b);
        write_body_v3(output_stream, key, game_title);
        const auto params = parse_params_file(output_stream);
        REQUIRE(params.key == key);
        REQUIRE(params.game_title == game_title);
    }

    SECTION("Version 4")
    {
        const auto key = "abc"_b;
        const auto game_title = "some generic title"_b;
        io::MemoryByteStream output_stream;
        output_stream.write("[SCR-PARAMS]v04"_b);
        write_body_v3(output_stream, key, game_title);
        const auto params = parse_params_file(output_stream);
        REQUIRE(params.key == key);
        REQUIRE(params.game_title == game_title);
    }

    SECTION("Version 5")
    {
        const auto key = "abc"_b;
        const auto game_title = "some generic title"_b;
        io::MemoryByteStream output_stream;
        output_stream.write("[SCR-PARAMS]v05"_b);
        write_body_v5(output_stream, key, game_title);
        const auto params = parse_params_file(output_stream);
        REQUIRE(params.key == key);
        REQUIRE(params.game_title == game_title);
    }

    SECTION("Version 5.1")
    {
        const auto key = "abc"_b;
        const auto game_title = "some generic title"_b;
        io::MemoryByteStream output_stream;
        output_stream.write("[SCR-PARAMS]v05.1"_b);
        write_body_v5(output_stream, key, game_title);
        const auto params = parse_params_file(output_stream);
        REQUIRE(params.key == key);
        REQUIRE(params.game_title == game_title);
    }

    SECTION("Version 5.2")
    {
        const auto key = "abc"_b;
        const auto game_title = "some generic title"_b;
        io::MemoryByteStream output_stream;
        output_stream.write("[SCR-PARAMS]v05.2"_b);
        write_body_v5_2(output_stream, key, game_title);
        const auto params = parse_params_file(output_stream);
        REQUIRE(params.key == key);
        REQUIRE(params.game_title == game_title);
    }

    SECTION("Version 5.3")
    {
        const auto key = "abc"_b;
        const auto game_title = "some generic title"_b;
        io::MemoryByteStream output_stream;
        output_stream.write("[SCR-PARAMS]v05.3"_b);
        write_body_v5_3(output_stream, key, game_title);
        const auto params = parse_params_file(output_stream);
        REQUIRE(params.key == key);
        REQUIRE(params.game_title == game_title);
    }
}
