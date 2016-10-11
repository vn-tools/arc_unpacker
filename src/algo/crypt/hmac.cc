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

#include "algo/crypt/hmac.h"
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include "err.h"

using namespace au;
using namespace au::algo::crypt;

bstr algo::crypt::hmac(
    const bstr &input, const bstr &key, const HmacKind hmac_kind)
{
    const EVP_MD *md;
    if (hmac_kind == HmacKind::Sha512)
        md = EVP_sha512();
    else
        throw err::NotSupportedError("Unimplemented hash kind");

    unsigned int final_size;
    bstr output(EVP_MAX_MD_SIZE);

    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key.get<const u8>(), key.size(), md, NULL);
    HMAC_Update(&ctx, input.get<const u8>(), input.size());
    HMAC_Final(&ctx, output.get<u8>(), &final_size);
    HMAC_CTX_cleanup(&ctx);

    return output.substr(0, final_size);
}
