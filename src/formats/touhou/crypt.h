#ifndef FORMATS_TOUHOU_CRYPT_H
#define FORMATS_TOUHOU_CRYPT_H
#include "io/io.h"

namespace Formats
{
    namespace Touhou
    {
        typedef struct DecryptorContext
        {
            u8 key;
            u8 step;
            size_t block_size;
            size_t limit;

            bool operator ==(const DecryptorContext &other) const;
        } DecryptorContext;

        void decrypt(
            IO &input,
            size_t size,
            IO &output,
            const DecryptorContext &context);
    }
}

#endif
