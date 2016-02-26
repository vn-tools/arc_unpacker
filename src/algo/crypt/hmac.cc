#include "algo/crypt/hmac.h"
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include "err.h"

using namespace au;
using namespace au::algo::crypt;

bstr algo::crypt::hmac(
    const bstr &input, const bstr &key, const HmacKind hmac_kind)
{
    const EVP_MD *md;
    if (hmac_kind == HmacKind::Sha512)
        md = EVP_sha512();
    else
        throw err::NotSupportedError("Unimplemented hash kind");

    unsigned int final_size;
    bstr output(EVP_MAX_MD_SIZE);

    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key.get<const u8>(), key.size(), md, NULL);
    HMAC_Update(&ctx, input.get<const u8>(), input.size());
    HMAC_Final(&ctx, output.get<u8>(), &final_size);
    HMAC_CTX_cleanup(&ctx);

    return output.substr(0, final_size);
}
