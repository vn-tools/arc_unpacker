#ifndef FORMATS_KIRIKIRI_TLG_LZSS_COMPRESSOR_H
#define FORMATS_KIRIKIRI_TLG_LZSS_COMPRESSOR_H
#include <memory>
#include "types.h"

namespace Formats
{
    namespace Kirikiri
    {
        namespace Tlg
        {
            class LzssDecompressor
            {
            public:
                LzssDecompressor();
                ~LzssDecompressor();

                void init_dictionary(u8 dictionary[4096]);

                void decompress(
                    u8 *input, size_t input_size,
                    u8 *output, size_t output_size);
            private:
                struct Priv;
                std::unique_ptr<Priv> p;
            };
        }
    }
}

#endif
