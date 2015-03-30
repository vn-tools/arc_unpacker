#ifndef FORMATS_TOUHOU_PAK2_IMAGE_CONVERTER_H
#define FORMATS_TOUHOU_PAK2_IMAGE_CONVERTER_H
#include <boost/filesystem/path.hpp>
#include <map>
#include <cstdint>
#include "formats/converter.h"

namespace Formats
{
    namespace Touhou
    {
        typedef std::map<boost::filesystem::path, std::unique_ptr<uint32_t[]>>
            PaletteMap;

        class Pak2ImageConverter : public Converter
        {
        public:
            Pak2ImageConverter(PaletteMap &palette_map);
            ~Pak2ImageConverter();
            void decode_internal(File &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
