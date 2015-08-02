#ifndef AU_FMT_TOUHOU_CRYPT_H
#define AU_FMT_TOUHOU_CRYPT_H
#include "io/io.h"

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

    void decrypt(
        io::IO &input,
        size_t size,
        io::IO &output,
        const DecryptorContext &context);

} } }

#endif
