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

#include "types.h"
#include <limits>
#include "test_support/catch.h"

using namespace au;

TEST_CASE("Type properties", "[core][types]")
{
    REQUIRE(std::numeric_limits<f32>::is_iec559);
    REQUIRE(std::numeric_limits<f64>::is_iec559);
    REQUIRE(std::numeric_limits<f32>::digits == 24);
    REQUIRE(std::numeric_limits<f64>::digits == 53);
}

TEST_CASE("bstr", "[core][types]")
{
    SECTION("Constructors")
    {
        SECTION("Empty")
        {
            const bstr x;
            REQUIRE(x == ""_b);
            REQUIRE(x.size() == 0);
        }

        SECTION("Reservation")
        {
            const bstr x(6);
            REQUIRE(x == "\x00\x00\x00\x00\x00\x00"_b);
            REQUIRE(x.size() == 6);
        }

        SECTION("Reservation + fill")
        {
            const bstr y(6, '\xFF');
            REQUIRE(y == "\xFF\xFF\xFF\xFF\xFF\xFF"_b);
            REQUIRE(y.size() == 6);
        }

        SECTION("Plain copy constructor")
        {
            const bstr x("test\x00\x01"_b);
            const bstr y(x);
            REQUIRE(y == x);
        }

        SECTION("Copying from C string")
        {
            const bstr x("test\x00\x01", 6);
            REQUIRE(x == "test\x00\x01"_b);
            REQUIRE(x.size() == 6);
        }

        SECTION("Copying from unsigned C string")
        {
            const u8 a[] = {'t', 'e', 's', 't', 0xFF};
            const bstr x(a, 5);
            REQUIRE(x == "test\xFF"_b);
            REQUIRE(x.size() == 5);
        }

        SECTION("Copying from C++ string")
        {
            const bstr x(std::string("test\x00\x01", 6));
            REQUIRE(x == "test\x00\x01"_b);
            REQUIRE(x.size() == 6);
            const bstr y(std::string("test\x00\x01"));
            REQUIRE(y == "test"_b);
            REQUIRE(y.size() == 4);
        }
    }

    SECTION("Size and emptiness")
    {
        bstr x;
        REQUIRE(x.size() == 0);
        REQUIRE(x.empty());
        x += '1';
        REQUIRE(x.size() == 1);
        REQUIRE(!x.empty());
    }

    SECTION("Converting to C++ string")
    {
        const bstr x("test\x00\x01", 6);
        REQUIRE(x.str().size() == 6);
        REQUIRE(x.str() == std::string("test\x00\x01", 6));
        const bstr y("test\x00\x01", 6);
        REQUIRE(y.str(true).size() == 4);
        REQUIRE(y.str(true) == std::string("test", 4));
        const bstr z("unterminated", 12);
        REQUIRE(z.str(true).size() == 12);
        REQUIRE(z.str(true) == std::string("unterminated", 12));
    }

    SECTION("Searching for substrings")
    {
        const bstr x("test\x00\x01", 6);
        REQUIRE(x.find("y"_b) == bstr::npos);
        REQUIRE(x.find("e"_b) != bstr::npos);
        REQUIRE(x.find("e"_b) == 1);
        REQUIRE(x.find("e"_b, 1) == 1);
        REQUIRE(x.find("e"_b, 2) == bstr::npos);
        REQUIRE(x.find("\x00\x01"_b) == 4);
        REQUIRE(x.find("\x00\x01"_b) == 4);
    }

    SECTION("Replacing substrings")
    {
        SECTION("Inside bounds")
        {
            auto x = "test\x00\x01"_b;
            x.replace(2, 2, "x"_b);
            REQUIRE(x == "tex\x00\x01"_b);
        }
        SECTION("Offset out of bounds")
        {
            auto x = "test\x00\x01"_b;
            x.replace(7, 0, "x"_b);
            REQUIRE(x == "test\x00\x01x"_b);
        }
        SECTION("Size out of bounds")
        {
            auto x = "test\x00\x01"_b;
            x.replace(5, 2, "x"_b);
            REQUIRE(x == "test\x00x"_b);
        }
        SECTION("Negative offset")
        {
            auto x = "test\x00\x01"_b;
            x.replace(-1, 1, "x"_b);
            REQUIRE(x == "test\x00x"_b);
        }
        SECTION("Negative size")
        {
            auto x = "test\x00\x01"_b;
            x.replace(1, -1, "x"_b);
            REQUIRE(x == "tx"_b);
        }
    }

    SECTION("Extracting substrings")
    {
        const bstr x("test\x00\x01", 6);
        SECTION("Plain substring")
        {
            REQUIRE(x.substr(0, 6) == x);
            REQUIRE(x.substr(0, 5) == "test\x00"_b);
            REQUIRE(x.substr(1, 5) == "est\x00\x01"_b);
            REQUIRE(x.substr(1) == "est\x00\x01"_b);
            REQUIRE(x.substr(2) == "st\x00\x01"_b);
            REQUIRE(x.substr(1, 0) == ""_b);
        }
        SECTION("Offset out of bounds")
        {
            REQUIRE(x.substr(7) == ""_b);
        }
        SECTION("Size out of bounds")
        {
            REQUIRE(x.substr(0, 7) == "test\x00\x01"_b);
            REQUIRE(x.substr(1, 7) == "est\x00\x01"_b);
        }
        SECTION("Negative offsets")
        {
            REQUIRE(x.substr(-1) == "\x01"_b);
            REQUIRE(x.substr(-1, 0) == ""_b);
            REQUIRE(x.substr(-1, 1) == "\x01"_b);
            REQUIRE(x.substr(-2, 1) == "\x00"_b);
            REQUIRE(x.substr(-2, 2) == "\x00\x01"_b);
        }
        SECTION("Negative sizes")
        {
            REQUIRE(x.substr(1, -1) == "est\x00\x01"_b);
        }
    }

    SECTION("Resizing")
    {
        bstr x = "\x01\x02"_b;
        x.resize(2);
        REQUIRE(x == "\x01\x02"_b);
        REQUIRE(x.capacity() >= 2);
        x.resize(4);
        REQUIRE(x == "\x01\x02\x00\x00"_b);
        REQUIRE(x.capacity() >= 4);
        x.resize(1);
        REQUIRE(x == "\x01"_b);
        REQUIRE(x.capacity() >= 1);
    }

    SECTION("Reserving")
    {
        bstr x = "\x01\x02"_b;
        x.reserve(2);
        REQUIRE(x == "\x01\x02"_b);
        REQUIRE(x.capacity() >= 2);
        x.reserve(4);
        REQUIRE(x == "\x01\x02"_b);
        REQUIRE(x.capacity() >= 4);
        x.reserve(1);
        REQUIRE(x == "\x01\x02"_b);
        REQUIRE(x.capacity() >= 4);
    }

    SECTION("Concatenating")
    {
        const bstr x = "\x00\x01"_b;
        const bstr y = "\x00\x02"_b;
        const bstr z = x + y;
        REQUIRE(z == "\x00\x01\x00\x02"_b);
    }

    SECTION("Appending")
    {
        SECTION("bstr")
        {
            const bstr x = "\x00\x01"_b;
            const bstr y = "\x00\x02"_b;
            bstr z = ""_b;
            REQUIRE(z == ""_b);
            z += x;
            z += y;
            REQUIRE(z == "\x00\x01\x00\x02"_b);
        }

        SECTION("bytes")
        {
            bstr z = ""_b;
            REQUIRE(z == ""_b);
            z += 'A';
            z += static_cast<u8>('\xFF');
            REQUIRE(z == "A\xFF"_b);
        }
    }

    SECTION("Testing for equality and inequality")
    {
        SECTION("Equality")
        {
            SECTION("Plain")
            {
                const bstr x = "a"_b;
                const bstr y = "a"_b;
                REQUIRE(x == x);
                REQUIRE(x == y);
                REQUIRE(y == x);
                REQUIRE(y == y);
            }
            SECTION("NULL terminated")
            {
                const bstr x = "\x00\x01"_b;
                const bstr y = "\x00\x01"_b;
                REQUIRE(x == x);
                REQUIRE(x == y);
                REQUIRE(y == x);
                REQUIRE(y == y);
            }
        }

        SECTION("Inequality")
        {
            SECTION("Plain")
            {
                const bstr x = "a"_b;
                const bstr y = "b"_b;
                REQUIRE(x != y);
                REQUIRE(y != x);
            }
            SECTION("NULL terminated")
            {
                const bstr x = "\x00\x01"_b;
                const bstr y = "\x00\x00"_b;
                REQUIRE(x != y);
                REQUIRE(y != x);
            }
        }
    }

    SECTION("Comparing bstr")
    {
        bstr x, y("1"), z("2"_b);
        REQUIRE(x < y); REQUIRE(!(y < x));
        REQUIRE(x < z); REQUIRE(!(z < x));
        REQUIRE(y < z); REQUIRE(!(z < y));
        REQUIRE(x <= y); REQUIRE(!(y <= x));
        REQUIRE(x <= z); REQUIRE(!(z <= x));
        REQUIRE(y <= z); REQUIRE(!(z <= y));
        REQUIRE(y > x); REQUIRE(!(x > y));
        REQUIRE(z > x); REQUIRE(!(x > z));
        REQUIRE(z > y); REQUIRE(!(y > z));
        REQUIRE(y >= x); REQUIRE(!(x >= y));
        REQUIRE(z >= x); REQUIRE(!(x >= z));
        REQUIRE(z >= y); REQUIRE(!(y >= z));
        REQUIRE(y <= y);
        REQUIRE(y >= y);
    }

    SECTION("Accessing individual characters")
    {
        SECTION("Via index operator")
        {
            bstr x = "\x00\x01"_b;
            REQUIRE(x[0] == '\x00');
            REQUIRE(x[1] == '\x01');
            x[0] = 0x0F;
            REQUIRE(x[0] == '\x0F');
        }

        SECTION("Via .at() method")
        {
            const bstr x = "\x00\x01"_b;
            SECTION("Plain")
            {
                REQUIRE(x.at(0) == 0);
                REQUIRE(x.at(1) == 1);
            }
            SECTION("Out of bounds")
            {
                REQUIRE_THROWS(x.at(2));
                REQUIRE_THROWS(x.at(-1));
            }
        }
    }

    SECTION("Retrieving pointers")
    {
        SECTION("Const pointers to the beginning of data")
        {
            const bstr x = "\x00\x01"_b;
            REQUIRE(x.get<const char>()[0] == 0);
            REQUIRE(x.get<const char>()[1] == 1);
            REQUIRE(bstr(x.get<const char>(), 2) == "\x00\x01"_b);
        }

        SECTION("Non-const pointers to the beginning of data")
        {
            bstr x(3);
            x.get<u16>()[0] = 1;
            x.get<u8>()[2] = 2;
            REQUIRE(x == "\x01\x00\x02"_b);
        }

        SECTION("Pointers to the end of data")
        {
            const bstr x = "\x00\x01"_b;
            REQUIRE(x.end<const char>()[-1] == 1);
            REQUIRE(static_cast<bool>(
                x.end<const char>() == x.get<const char>() + 2));
            REQUIRE(static_cast<bool>(
                x.end<const u16>() == &x.get<const u16>()[1]));
            REQUIRE(x.end<const u16>()[-1] == 0x100);

            // align the boundary to the sizeof(T)
            const bstr y = "\x00\x01\x02"_b;
            REQUIRE(static_cast<bool>(
                y.end<const u16>() == &y.get<const u16>()[1]));
            REQUIRE(y.end<const u16>()[-1] == 0x100);
        }
    }

    SECTION("Iterating over bstr")
    {
        SECTION("Non const")
        {
            bstr tmp = "123"_b;
            for (auto &c : tmp)
                c++;
            REQUIRE(tmp == "234"_b);
        }
        SECTION("Const")
        {
            bstr tmp = ""_b;
            for (const auto c : "123"_b)
            {
                tmp += c;
                tmp += "|"_b;
            }
            REQUIRE(tmp == "1|2|3|"_b);
        }
    }
}
