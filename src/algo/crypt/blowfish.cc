#include "algo/crypt/blowfish.h"
#include <cstring>
#include <openssl/blowfish.h>
#include "err.h"
#include "types.h"

using namespace au;
using namespace au::algo::crypt;

struct Blowfish::Priv final
{
    Priv(const bstr &key_str);
    ~Priv();

    std::unique_ptr<BF_KEY> key;
};

Blowfish::Priv::Priv(const bstr &key_str) : key(new BF_KEY)
{
    BF_set_key(
        key.get(),
        key_str.size(),
        key_str.get<const u8>());
}

Blowfish::Priv::~Priv()
{
}

Blowfish::Blowfish(const bstr &key) : p(new Priv(key))
{
}

Blowfish::~Blowfish()
{
}

size_t Blowfish::block_size()
{
    return BF_BLOCK;
}

bstr Blowfish::decrypt(const bstr &input) const
{
    bstr output(input);
    decrypt_in_place(output);
    return output;
}

void Blowfish::decrypt_in_place(bstr &input) const
{
    size_t left = input.size() / BF_BLOCK;
    size_t pos = 0;

    while (left)
    {
        BF_decrypt(&input.get<BF_LONG>()[pos], p->key.get());
        left--;
        pos += BF_BLOCK / sizeof(BF_LONG);
    }
}

bstr Blowfish::encrypt(const bstr &input) const
{
    size_t left = input.size();
    size_t done = 0;

    bstr output;

    while (left)
    {
        size_t block_size = BF_BLOCK;
        if (left < block_size)
            block_size = left;
        BF_LONG transit[2] = {0, 0};
        std::memcpy(&transit[0], &input.get<u8>()[done], block_size);
        BF_encrypt(transit, p->key.get());
        output += bstr(reinterpret_cast<char*>(&transit), BF_BLOCK);
        left -= block_size;
        done += block_size;
    }

    return output;
}
