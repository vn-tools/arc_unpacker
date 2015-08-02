#include "fmt/touhou/palette.h"
#include "util/colors.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

Palette au::fmt::touhou::create_default_palette()
{
    Palette default_palette;
    for (auto i : util::range(256))
        default_palette[i] = util::color::rgba_gray(i);
    return default_palette;
}
