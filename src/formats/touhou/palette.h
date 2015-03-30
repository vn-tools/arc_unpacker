#ifndef FORMATS_TOUHOU_PALETTE_H
#define FORMATS_TOUHOU_PALETTE_H
#include <array>
#include <boost/filesystem/path.hpp>
#include <map>

namespace Formats
{
    namespace Touhou
    {
        typedef std::array<uint32_t, 256> Palette;

        typedef std::map<boost::filesystem::path, Palette> PaletteMap;

        Palette create_default_palette();
    }
}

#endif
