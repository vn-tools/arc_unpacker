#ifndef AU_FMT_TOUHOU_PALETTE_MAP_H
#define AU_FMT_TOUHOU_PALETTE_MAP_H
#include <map>
#include <memory>
#include "pix/palette.h"

namespace au {
namespace fmt {
namespace touhou {

    using PaletteMap = std::map<std::string, std::shared_ptr<pix::Palette>>;

} } }

#endif
