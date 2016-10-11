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

#include "algo/crypt/md5.h"
#include <openssl/md5.h>

using namespace au;

bstr algo::crypt::md5(const bstr &input)
{
    MD5_CTX ctx;
    MD5_Init(&ctx);
    u8 output[MD5_DIGEST_LENGTH];
    MD5_Update(&ctx, input.get<const u8>(), input.size());
    MD5_Final(output, &ctx);
    return bstr(output, MD5_DIGEST_LENGTH);
}

bstr algo::crypt::md5(
    const bstr &input,
    const std::array<u32, 4> &custom_init)
{
    MD5_CTX ctx;
    MD5_Init(&ctx);
    ctx.A = custom_init[0];
    ctx.B = custom_init[1];
    ctx.C = custom_init[2];
    ctx.D = custom_init[3];
    u8 output[MD5_DIGEST_LENGTH];
    MD5_Update(&ctx, input.get<const u8>(), input.size());
    MD5_Final(output, &ctx);
    return bstr(output, MD5_DIGEST_LENGTH);
}
