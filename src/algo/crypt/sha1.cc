#include "algo/crypt/sha1.h"
#include <openssl/sha.h>

using namespace au;

bstr algo::crypt::sha1(const bstr &input)
{
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    u8 output[SHA_DIGEST_LENGTH];
    SHA1_Update(&ctx, input.get<const u8>(), input.size());
    SHA1_Final(output, &ctx);
    return bstr(output, SHA_DIGEST_LENGTH);
}
