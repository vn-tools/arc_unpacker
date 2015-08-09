#include "util/crypt/blowfish.h"

using namespace au::util::crypt;

struct Blowfish::Priv
{
};

Blowfish::Blowfish(const bstr &key)
{
    throw std::runtime_error(
        "Blowfish is unavailable - need to compile with OpenSSL.");
}

Blowfish::~Blowfish()
{
}

size_t Blowfish::block_size()
{
    return 8;
}

bstr Blowfish::decrypt(const bstr &input) const
{
    return ""_b;
}

bstr Blowfish::encrypt(const bstr &input) const
{
    return "";
}

std::string Blowfish::encrypt(const std::string &input) const
{
    return "";
}
