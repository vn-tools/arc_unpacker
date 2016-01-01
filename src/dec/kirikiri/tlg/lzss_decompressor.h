#pragma once

#include <memory>
#include "types.h"

namespace au {
namespace dec {
namespace kirikiri {
namespace tlg {

    class LzssDecompressor final
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
