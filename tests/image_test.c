#include <stdlib.h>
#include "assert_ex.h"
#include "image.h"
#include "io.h"

int main(void)
{
    IO *io = io_create_from_file("tests/test_files/gfx/reimu_transparent.png", "rb");
    assert_not_null(io);

    char *data = io_read_string(io, io_size(io));
    assert_not_null(data);

    Image *image = image_create_from_boxed(data, io_size(io));
    assert_not_null(image);

    assert_equali(641, image_width(image));
    assert_equali(720, image_height(image));

    size_t pixel_pos = image_width(image) * 100 + 200;
    uint8_t r = ((uint8_t*)image_pixel_data(image))[pixel_pos * 4];
    uint8_t g = ((uint8_t*)image_pixel_data(image))[pixel_pos * 4 + 1];
    uint8_t b = ((uint8_t*)image_pixel_data(image))[pixel_pos * 4 + 2];
    uint8_t a = ((uint8_t*)image_pixel_data(image))[pixel_pos * 4 + 3];
    assert_equali(0xfe, r);
    assert_equali(0x0a, g);
    assert_equali(0x17, b);
    assert_equali(0xff, a);

    free(data);
    io_destroy(io);
    return 0;
}
