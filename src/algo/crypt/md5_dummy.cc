#include "algo/crypt/md5.h"
#include "err.h"

using namespace au;

bstr algo::crypt::md5(const bstr &input)
{
    throw err::NotSupportedError(
        "MD5 is unavailable - need to compile with OpenSSL.");
}

bstr algo::crypt::md5(
    const bstr &input,
    const std::array<u32, 4> &custom_init)
{
    throw err::NotSupportedError(
        "MD5 is unavailable - need to compile with OpenSSL.");
}
