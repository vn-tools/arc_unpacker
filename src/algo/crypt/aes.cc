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

#include "algo/crypt/aes.h"
#include <openssl/evp.h>
#include <stdexcept>

using namespace au;

bstr algo::crypt::aes256_decrypt_cbc(
    const bstr &input, const bstr &iv, const bstr &key)
{
    if (key.size() != 32)
        throw std::logic_error("Invalid key size");
    if (iv.size() != 16)
        throw std::logic_error("Invalid IV size");

    bstr output(input.size());
    EVP_CIPHER_CTX ctx;
    EVP_DecryptInit(
        &ctx,
        EVP_aes_256_cbc(),
        key.get<const u8>(),
        iv.get<const u8>());

    int actual_size = 0;
    EVP_DecryptUpdate(
        &ctx,
        output.get<u8>(),
        &actual_size,
        input.get<const u8>(),
        input.size());

    int final_size;
    EVP_DecryptFinal(&ctx, output.get<u8>() + actual_size, &final_size);
    actual_size += final_size;
    output.resize(actual_size);
    return output;
}

bstr algo::crypt::aes256_encrypt_cbc(
    const bstr &input, const bstr &iv, const bstr &key)
{
    if (key.size() != 32)
        throw std::logic_error("Invalid key size");
    if (iv.size() != 16)
        throw std::logic_error("Invalid IV size");

    bstr output(input.size() + 0x10);
    EVP_CIPHER_CTX ctx;
    EVP_EncryptInit(
        &ctx,
        EVP_aes_256_cbc(),
        key.get<const u8>(),
        iv.get<const u8>());

    int actual_size = 0;
    EVP_EncryptUpdate(
        &ctx,
        output.get<u8>(),
        &actual_size,
        input.get<const u8>(),
        input.size());

    int final_size;
    EVP_EncryptFinal(&ctx, output.get<u8>() + actual_size, &final_size);
    actual_size += final_size;
    output.resize(actual_size);
    return output;
}
