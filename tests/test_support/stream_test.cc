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

#include "test_support/catch.h"
#include "test_support/common.h"
#include "test_support/stream_test.h"

using namespace au;

void tests::stream_test(
    const std::function<std::unique_ptr<io::BaseByteStream>()> &factory,
    const std::function<void()> &cleanup)
{
    {
        auto stream = factory();

        SECTION("Proper file position after initialization")
        {
            REQUIRE(stream->size() == 0);
            REQUIRE(stream->pos() == 0);
        }

        SECTION("NULL bytes in binary data don't cause anomalies")
        {
            stream->write("\x00\x00\x00\x01"_b).seek(0);
            REQUIRE(stream->size() == 4);
            REQUIRE(stream->read_le<u32>() == 0x01000000);
            stream->seek(0);
            stream->write("\x00\x00\x00\x02"_b);
            stream->seek(0);
            REQUIRE(stream->read_le<u32>() == 0x02000000);
        }

        SECTION("Reading integers")
        {
            stream->write("\x01\x00\x00\x00"_b).seek(0);
            stream->seek(0);
            REQUIRE(stream->read_le<u32>() == 1);
            REQUIRE(stream->size() == 4);
        }

        SECTION("Writing integers")
        {
            stream->write_le<u32>(1);
            stream->seek(0);
            REQUIRE(stream->read_le<u32>() == 1);
            REQUIRE(stream->size() == 4);
        }

        SECTION("Skipping and telling position")
        {
            stream->write("\x01\x0F\x00\x00"_b).seek(0);

            SECTION("Positive offset")
            {
                stream->skip(1);
                REQUIRE(stream->pos() == 1);
                REQUIRE(stream->read_le<u16>() == 15);
                REQUIRE(stream->pos() == 3);
            }

            SECTION("Negative offset")
            {
                stream->read(2);
                stream->skip(-1);
                REQUIRE(stream->read_le<u16>() == 15);
                REQUIRE(stream->pos() == 3);
            }
        }

        SECTION("Seeking and telling position")
        {
            stream->write("\x01\x00\x00\x00"_b).seek(0);

            SECTION("Initial seeking")
            {
                REQUIRE(stream->pos() == 0);
            }

            SECTION("Seeking to EOF")
            {
                stream->seek(4);
                REQUIRE(stream->pos() == 4);
            }

            SECTION("Seeking beyond EOF throws errors")
            {
                REQUIRE(stream->size() == 4);
                stream->seek(3);
                REQUIRE_THROWS(stream->seek(5));
                REQUIRE(stream->pos() == 3);
            }

            SECTION("Seeking affects what gets read")
            {
                stream->seek(1);
                REQUIRE(stream->read_le<u16>() == 0);
                stream->seek(0);
                REQUIRE(stream->read_le<u32>() == 1);
            }

            SECTION("Seeking affects telling position")
            {
                stream->seek(2);
                REQUIRE(stream->pos() == 2);
                stream->skip(1);
                REQUIRE(stream->pos() == 3);
                stream->skip(-1);
                REQUIRE(stream->pos() == 2);
            }
        }

        SECTION("Reading NULL-terminated strings")
        {
            stream->write("abc\x00""def\x00"_b).seek(0);
            tests::compare_binary(stream->read_to_zero(), "abc"_b);
            tests::compare_binary(stream->read_to_zero(), "def"_b);
        }

        SECTION("Reading lines")
        {
            stream->write("line1\nline2\n"_b).seek(0);
            tests::compare_binary(stream->read_line(), "line1"_b);
            tests::compare_binary(stream->read_line(), "line2"_b);
        }

        SECTION("Reading unterminated lines")
        {
            stream->write("line"_b).seek(0);
            tests::compare_binary(stream->read_line(), "line"_b);
        }

        SECTION("Reading NULL-terminated lines")
        {
            stream->write("line\x00"_b).seek(0);
            tests::compare_binary(stream->read_line(), "line"_b);
        }

        SECTION("Reading lines containing carriage returns")
        {
            stream->write("line1\r\nline2\r\n"_b).seek(0);
            tests::compare_binary(stream->read_line(), "line1"_b);
            tests::compare_binary(stream->read_line(), "line2"_b);
        }

        SECTION("Reading strings")
        {
            stream->write("abc\x00"_b).seek(0);
            const auto result = stream->read(2);
            tests::compare_binary(result, "ab"_b);
        }

        SECTION("Writing strings")
        {
            stream->write("abc\x00"_b).seek(0);
            stream->write("xy"_b);
            stream->skip(-2);
            const auto result = stream->read(3);
            tests::compare_binary(result, "xyc"_b);
        }

        SECTION("Reading integers with endianness conversions")
        {
            stream->write("\x12\x34\x56\x78"_b).seek(0);
            REQUIRE(stream->read<u8>() == 0x12); stream->skip(-1);
            REQUIRE(stream->read_le<u16>() == 0x3412); stream->skip(-2);
            REQUIRE(stream->read_be<u16>() == 0x1234); stream->skip(-2);
            REQUIRE(stream->read_le<u32>() == 0x78563412); stream->skip(-4);
            REQUIRE(stream->read_be<u32>() == 0x12345678); stream->skip(-4);
        }
    }

    cleanup();
}
