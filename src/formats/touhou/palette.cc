#include "formats/touhou/palette.h"
#include "util/colors.h"

using namespace au;
using namespace au::fmt::touhou;

Palette au::fmt::touhou::create_default_palette()
{
    Palette default_palette;
    for (size_t i = 0; i < 256; i++)
        default_palette[i] = util::color::rgba_gray(i);
    return default_palette;
}
