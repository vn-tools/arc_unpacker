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

#include "algo/crypt/camellia.h"
#include "test_support/catch.h"

using namespace au;
using namespace au::algo::crypt;

TEST_CASE("Camellia", "[algo][crypt]")
{
    Camellia c({
        0x364F9A3E, 0x873B57D7, 0x920C8F7C, 0x2CCCC422,
        0x0309410A, 0xC7DFDB3F, 0x3180EB15, 0xE5D35D62,
        0x3FA9D12A, 0x34EF8ECB, 0x8A62FA9C, 0x537EB987,
        0x502947E1, 0x3A65CB88, 0x85A91735, 0x87E77D3D,
        0xB563DBCA, 0x1B009542, 0x16573462, 0x84C47A65,
        0x057C13F5, 0xC6598E67, 0xB444FBF1, 0x6157D19F,
        0x77ED2698, 0xE6E57501, 0xEACADAAD, 0xAAD676B2,
        0x266968F1, 0xEC566ACE, 0x261B7E2E, 0x1C46FE7C,
        0x8AA8675B, 0x6CF5157A, 0xC1717A59, 0x36982E47,
        0x5D4C7804, 0xE2E976F7, 0x7592E4C7, 0x304A48B3,
        0xC8E3D3FF, 0xFF8B759A, 0x9EF24637, 0xC98FE507,
        0x04767A7A, 0x693DB6A7, 0x084FAE8D, 0x90DBB24A,
        0x2CAA8502, 0x4DDBE69D, 0x9CF7D7AF, 0xF3D06D0A,
    });

    u32 input_block[4], output_block[4], actual_block[4];

    input_block[0] = 0x12345678;
    input_block[1] = 0x23456789;
    input_block[2] = 0x34567890;
    input_block[3] = 0x45678901;

    c.encrypt_block_128(0, input_block, output_block);
    c.decrypt_block_128(0, output_block, actual_block);

    REQUIRE(actual_block[0] == input_block[0]);
    REQUIRE(actual_block[1] == input_block[1]);
    REQUIRE(actual_block[2] == input_block[2]);
    REQUIRE(actual_block[3] == input_block[3]);
}
