#include <stdlib.h>
#include "assert_ex.h"
#include "image.h"
#include "io.h"

void test_transparent()
{
    IO *io = io_create_from_file(
        "tests/test_files/gfx/reimu_transparent.png", "rb");
    assert_not_null(io);

    Image *image = image_create_from_boxed(io);
    assert_not_null(image);

    assert_equali(641, image_width(image));
    assert_equali(720, image_height(image));
    assert_equali(641 * 720 * 4, image_pixel_data_size(image));

    size_t pixel_pos = image_width(image) * 100 + 200;
    uint8_t r = ((uint8_t*)image_pixel_data(image))[pixel_pos * 4];
    uint8_t g = ((uint8_t*)image_pixel_data(image))[pixel_pos * 4 + 1];
    uint8_t b = ((uint8_t*)image_pixel_data(image))[pixel_pos * 4 + 2];
    uint8_t a = ((uint8_t*)image_pixel_data(image))[pixel_pos * 4 + 3];
    assert_equali(0xfe, r);
    assert_equali(0x0a, g);
    assert_equali(0x17, b);
    assert_equali(0xff, a);

    io_destroy(io);
}

void test_opaque()
{
    IO *io = io_create_from_file("tests/test_files/gfx/usagi_opaque.png", "rb");
    assert_not_null(io);

    Image *image = image_create_from_boxed(io);
    assert_not_null(image);

    assert_equali(640, image_width(image));
    assert_equali(480, image_height(image));
    assert_equali(640 * 480 * 3, image_pixel_data_size(image));

    size_t pixel_pos = image_width(image) * 100 + 200;
    uint8_t r = ((uint8_t*)image_pixel_data(image))[pixel_pos * 3];
    uint8_t g = ((uint8_t*)image_pixel_data(image))[pixel_pos * 3 + 1];
    uint8_t b = ((uint8_t*)image_pixel_data(image))[pixel_pos * 3 + 2];
    assert_equali(124, r);
    assert_equali(106, g);
    assert_equali(52, b);

    io_destroy(io);
}

int main(void)
{
    test_transparent();
    test_opaque();
    return 0;
}
