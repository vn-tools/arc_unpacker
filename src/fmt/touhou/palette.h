#ifndef AU_FMT_TOUHOU_PALETTE_H
#define AU_FMT_TOUHOU_PALETTE_H
#include <array>
#include <boost/filesystem/path.hpp>
#include <map>
#include "util/image.h"
#include "types.h"

namespace au {
namespace fmt {
namespace touhou {

    using Palette = std::array<util::Color, 256>;
    using PaletteMap = std::map<boost::filesystem::path, Palette>;
    Palette create_default_palette();

} } }

#endif
