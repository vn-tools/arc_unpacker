#ifndef FORMATS_TOUHOU_TFBM_IMAGE_CONVERTER_H
#define FORMATS_TOUHOU_TFBM_IMAGE_CONVERTER_H
#include <array>
#include <map>
#include <cstdint>
#include "formats/converter.h"

namespace Formats
{
    namespace Touhou
    {
        typedef std::array<uint32_t, 256> TfbmPalette;
        typedef std::map<std::string, TfbmPalette> TfbmPaletteMap;

        class TfbmConverter : public Converter
        {
        public:
            TfbmConverter();
            ~TfbmConverter();
            void set_palette_map(const TfbmPaletteMap &palette_map);
            void decode_internal(File &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
