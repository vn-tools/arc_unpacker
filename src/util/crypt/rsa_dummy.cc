#include "util/crypt/rsa.h"
#include "err.h"

using namespace au;
using namespace au::util::crypt;

struct Rsa::Priv final
{
};

Rsa::Rsa(const RsaKey &key)
{
    throw err::NotSupportedError(
        "RSA is unavailable - need to compile with OpenSSL.");
}

Rsa::~Rsa()
{
}

bstr Rsa::decrypt(const bstr &input) const
{
    return ""_b;
}
