#include <stdlib.h>
#include "assert_ex.h"
#include "image.h"
#include "io.h"
#include "test_support/converter_support.h"
#include "virtual_file.h"

static void compare_images(
    const Image *expected_image,
    const Image *actual_image)
{
    if (expected_image == NULL || actual_image == NULL)
    {
        assert_null(expected_image);
        assert_null(actual_image);
        return;
    }

    assert_equali(image_width(expected_image), image_width(actual_image));
    assert_equali(image_height(expected_image), image_height(actual_image));

    size_t x, y;
    for (y = 0; y < image_height(expected_image); y ++)
    {
        for (x = 0; x < image_width(expected_image); x ++)
        {
            uint32_t expected_rgba = image_color_at(expected_image, x, y);
            uint32_t actual_rgba = image_color_at(actual_image, x, y);
            assert_equali(expected_rgba, actual_rgba);
        }
    }
}

static Image *get_actual_image(const char *path, Converter *converter)
{
    IO *io = io_create_from_file(path, "rb");
    VirtualFile *file = virtual_file_create();
    io_write_string_from_io(file->io, io, io_size(io));
    converter_decode(converter, file);
    io_destroy(io);

    Image *image = image_create_from_boxed(file->io);
    assert_not_null(image);
    virtual_file_destroy(file);

    return image;
}

static Image *get_expected_image(const char *path)
{
    IO *io = io_create_from_file(path, "rb");
    Image *image = image_create_from_boxed(io);
    io_destroy(io);
    assert_not_null(image);
    return image;
}

void assert_decoded_image(
    Converter *converter,
    const char *path_to_input,
    const char *path_to_expected)
{
    assert_not_null(converter);
    assert_not_null(path_to_input);
    assert_not_null(path_to_expected);

    Image *actual_image = get_actual_image(path_to_input, converter);
    Image *expected_image = get_expected_image(path_to_expected);

    compare_images(expected_image, actual_image);

    image_destroy(actual_image);
    image_destroy(expected_image);
}
