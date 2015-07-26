#ifndef AU_FMT_TOUHOU_PALETTE_H
#define AU_FMT_TOUHOU_PALETTE_H
#include <array>
#include <boost/filesystem/path.hpp>
#include <map>
#include "types.h"

namespace au {
namespace fmt {
namespace touhou {

    typedef std::array<u32, 256> Palette;
    typedef std::map<boost::filesystem::path, Palette> PaletteMap;
    Palette create_default_palette();

} } }

#endif
