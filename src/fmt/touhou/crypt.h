#ifndef AU_FMT_TOUHOU_CRYPT_H
#define AU_FMT_TOUHOU_CRYPT_H
#include "types.h"

namespace au {
namespace fmt {
namespace touhou {

    struct DecryptorContext
    {
        u8 key;
        u8 step;
        size_t block_size;
        size_t limit;

        bool operator ==(const DecryptorContext &other) const;
    };

    bstr decrypt(const bstr &input, const DecryptorContext &context);

} } }

#endif
