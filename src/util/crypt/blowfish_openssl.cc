#include <cstring>
#include <openssl/blowfish.h>
#include "err.h"
#include "types.h"
#include "util/crypt/blowfish.h"

using namespace au;
using namespace au::util::crypt;

struct Blowfish::Priv final
{
    std::unique_ptr<BF_KEY> key;

    Priv(const bstr &key_str) : key(new BF_KEY)
    {
        BF_set_key(
            key.get(),
            key_str.size(),
            key_str.get<const u8>());
    }

    ~Priv()
    {
    }
};

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
    if (input.size() % BF_BLOCK != 0)
        throw err::BadDataSizeError();

    size_t left = input.size();
    size_t done = 0;

    bstr output;

    BF_LONG transit[2];
    while (left)
    {
        std::memcpy(transit, &input.get<u8>()[done], BF_BLOCK);
        BF_decrypt(transit, p->key.get());
        output += bstr(reinterpret_cast<char*>(&transit), BF_BLOCK);
        left -= BF_BLOCK;
        done += BF_BLOCK;
    }

    return output;
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
