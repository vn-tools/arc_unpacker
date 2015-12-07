#include "algo/crypt/md5.h"
#include <openssl/md5.h>

using namespace au;

bstr algo::crypt::md5(const bstr &input)
{
    MD5_CTX ctx;
    MD5_Init(&ctx);
    u8 output[MD5_DIGEST_LENGTH];
    MD5_Update(&ctx, input.get<const u8>(), input.size());
    MD5_Final(output, &ctx);
    return bstr(output, MD5_DIGEST_LENGTH);
}

bstr algo::crypt::md5(
    const bstr &input,
    const std::array<u32, 4> &custom_init)
{
    MD5_CTX ctx;
    MD5_Init(&ctx);
    ctx.A = custom_init[0];
    ctx.B = custom_init[1];
    ctx.C = custom_init[2];
    ctx.D = custom_init[3];
    u8 output[MD5_DIGEST_LENGTH];
    MD5_Update(&ctx, input.get<const u8>(), input.size());
    MD5_Final(output, &ctx);
    return bstr(output, MD5_DIGEST_LENGTH);
}
