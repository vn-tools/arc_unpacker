#include "fmt/touhou/palette.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

Palette au::fmt::touhou::create_default_palette()
{
    Palette default_palette;
    for (auto i : util::range(256))
    {
        default_palette[i].r = i;
        default_palette[i].b = i;
        default_palette[i].g = i;
        default_palette[i].a = 0xFF;
    }
    return default_palette;
}
