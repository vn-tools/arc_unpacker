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

#include "algo/binary.h"
#include "algo/range.h"
#include "err.h"

using namespace au;

// the name is because of -fno-operator-names
bstr algo::unxor(const bstr &input, const u8 key)
{
    bstr output(input);
    for (const auto i : algo::range(input.size()))
        output[i] ^= key;
    return output;
}

bstr algo::unxor(const bstr &input, const bstr &key)
{
    if (!key.size())
        throw err::BadDataSizeError();
    bstr output(input);
    for (const auto i : algo::range(input.size()))
        output[i] ^= key[i % key.size()];
    return output;
}
