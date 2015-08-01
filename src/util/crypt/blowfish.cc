#include <cstring>
#include <openssl/blowfish.h>
#include "types.h"
#include "util/crypt/blowfish.h"

using namespace au::util::crypt;

struct Blowfish::Priv
{
    std::unique_ptr<BF_KEY> key;

    Priv(const std::string &key_str) : key(new BF_KEY)
    {
        BF_set_key(
            key.get(),
            key_str.size(),
            reinterpret_cast<const u8*>(key_str.data()));
    }

    ~Priv()
    {
    }
};

Blowfish::Blowfish(const std::string &key) : p(new Priv(key))
{
}

Blowfish::~Blowfish()
{
}

size_t Blowfish::block_size()
{
    return BF_BLOCK;
}

std::string Blowfish::decrypt(const std::string &input) const
{
    if (input.size() % BF_BLOCK != 0)
        throw std::runtime_error("Unexpected input size");

    size_t left = input.size();
    size_t done = 0;

    std::string output;

    BF_LONG transit;
    while (left)
    {
        std::memcpy(&transit, input.data() + done, BF_BLOCK);
        BF_decrypt(&transit, p->key.get());
        output += std::string(reinterpret_cast<char*>(&transit), BF_BLOCK);
        left -= BF_BLOCK;
        done += BF_BLOCK;
    }

    return output;
}
