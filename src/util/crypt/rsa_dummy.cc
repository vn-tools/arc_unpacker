#include "util/crypt/rsa.h"

using namespace au;
using namespace au::util::crypt;

struct Rsa::Priv
{
};

Rsa::Rsa(const RsaKey &key)
{
    throw std::runtime_error(
        "RSA is unavailable - need to compile with OpenSSL.");
}

Rsa::~Rsa()
{
}

bstr Rsa::decrypt(const bstr &input) const
{
    return ""_b;
}
