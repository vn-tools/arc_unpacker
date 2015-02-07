#include <cassert>
#include <cstdlib>
#include "endian.h"
#include "file_io.h"
#include "formats/image.h"

void test_transparent()
{
    FileIO io("tests/test_files/gfx/reimu_transparent.png", "rb");

    std::unique_ptr<Image> image = Image::from_boxed(io);
    assert(image->width() == 641);
    assert(image->height() == 720);

    uint32_t rgba = image->color_at(200, 100);
    assert(rgba == be32toh(0xfe0a17ff));
}

void test_opaque()
{
    FileIO io("tests/test_files/gfx/usagi_opaque.png", "rb");

    std::unique_ptr<Image> image = Image::from_boxed(io);
    assert(image->width() == 640);
    assert(image->height() == 480);

    uint32_t rgba = image->color_at(200, 100);
    assert(rgba = be32toh(0x7c6a34ff));
}

int main(void)
{
    test_transparent();
    test_opaque();
    return 0;
}
