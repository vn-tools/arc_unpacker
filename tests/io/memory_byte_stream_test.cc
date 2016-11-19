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

#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/stream_test.h"

using namespace au;

TEST_CASE("MemoryByteStream", "[io][stream]")
{
    SECTION("Full test suite")
    {
        tests::stream_test(
            []() { return std::make_unique<io::MemoryByteStream>(); },
            []() { });
    }
}
