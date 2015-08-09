#include "util/crypt/blowfish.h"

using namespace au::util::crypt;

struct Blowfish::Priv
{
};

Blowfish::Blowfish(const std::string &key)
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

std::string Blowfish::decrypt(const std::string &input) const
{
    return "";
}

std::string Blowfish::encrypt(const std::string &input) const
{
    return "";
}
