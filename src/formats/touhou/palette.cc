#include "formats/touhou/palette.h"
#include "util/colors.h"
using namespace Formats::Touhou;

Palette Formats::Touhou::create_default_palette()
{
    Palette default_palette;
    for (size_t i = 0; i < 256; i++)
        default_palette[i] = rgba_gray(i);
    return default_palette;
}
