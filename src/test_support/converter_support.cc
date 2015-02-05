#include <cassert>
#include <cstring>
#include "formats/image.h"
#include "io.h"
#include "test_support/converter_support.h"
#include "virtual_file.h"

static void compare_images(
    const Image *expected_image,
    const Image *actual_image)
{
    if (expected_image == NULL || actual_image == NULL)
    {
        assert(expected_image == NULL);
        assert(actual_image == NULL);
        return;
    }

    assert(image_width(expected_image) == image_width(actual_image));
    assert(image_height(expected_image) == image_height(actual_image));

    size_t x, y;
    for (y = 0; y < image_height(expected_image); y ++)
    {
        for (x = 0; x < image_width(expected_image); x ++)
        {
            uint32_t expected_rgba = image_color_at(expected_image, x, y);
            uint32_t actual_rgba = image_color_at(actual_image, x, y);
            assert(expected_rgba == actual_rgba);
        }
    }
}

static Image *get_actual_image(const char *path, Converter *converter)
{
    IO *io = io_create_from_file(path, "rb");
    assert(io != NULL);

    VirtualFile *file = virtual_file_create();
    assert(file != NULL);

    if (!io_write_string_from_io(file->io, io, io_size(io)))
        assert(0);
    io_destroy(io);

    converter_decode(converter, file);

    Image *image = image_create_from_boxed(file->io);
    assert(image != NULL);
    virtual_file_destroy(file);

    return image;
}

static Image *get_expected_image(const char *path)
{
    IO *io = io_create_from_file(path, "rb");
    assert(io != NULL);

    Image *image = image_create_from_boxed(io);
    assert(image != NULL);
    io_destroy(io);

    return image;
}

void assert_decoded_image(
    Converter *converter,
    const char *path_to_input,
    const char *path_to_expected)
{
    assert(converter != NULL);
    assert(path_to_input != NULL);
    assert(path_to_expected != NULL);

    Image *actual_image = get_actual_image(path_to_input, converter);
    Image *expected_image = get_expected_image(path_to_expected);

    compare_images(expected_image, actual_image);

    image_destroy(actual_image);
    image_destroy(expected_image);
}
