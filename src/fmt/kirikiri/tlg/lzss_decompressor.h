#ifndef AU_FMT_KIRIKIRI_TLG_LZSS_COMPRESSOR_H
#define AU_FMT_KIRIKIRI_TLG_LZSS_COMPRESSOR_H
#include <memory>
#include "types.h"

namespace au {
namespace fmt {
namespace kirikiri {
namespace tlg {

    class LzssDecompressor
    {
    public:
        LzssDecompressor();
        ~LzssDecompressor();

        void init_dictionary(u8 dictionary[4096]);

        bstr decompress(const bstr &input, size_t output_size);
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }

#endif
