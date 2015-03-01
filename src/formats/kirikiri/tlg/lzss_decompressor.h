#ifndef FORMATS_KIRIKIRI_TLG_LZSS_COMPRESSOR_H
#define FORMATS_KIRIKIRI_TLG_LZSS_COMPRESSOR_H
#include <memory>

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

                void init_dictionary(unsigned char dictionary[4096]);

                void decompress(
                    unsigned char *input,
                    size_t input_size,
                    unsigned char *output,
                    size_t output_size);
            private:
                struct Internals;
                std::unique_ptr<Internals> internals;
            };
        }
    }
}


#endif
