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

#include "algo/crypt/sha1.h"
#include <openssl/sha.h>

using namespace au;

bstr algo::crypt::sha1(const bstr &input)
{
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    u8 output[SHA_DIGEST_LENGTH];
    SHA1_Update(&ctx, input.get<const u8>(), input.size());
    SHA1_Final(output, &ctx);
    return bstr(output, SHA_DIGEST_LENGTH);
}
