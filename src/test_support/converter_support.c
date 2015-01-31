#include <stdlib.h>
#include "assert_ex.h"
#include "image.h"
#include "io.h"
#include "test_support/converter_support.h"
#include "virtual_file.h"

void compare_images(
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

    assert_equali(
        image_pixel_data_size(expected_image),
        image_pixel_data_size(actual_image));

    assert_equalsn(
        image_pixel_data(expected_image),
        image_pixel_data(actual_image),
        image_pixel_data_size(expected_image));
}

void assert_decoded_image(
    Converter *converter,
    const char *path_to_input,
    const char *path_to_expected)
{
    assert_not_null(converter);
    assert_not_null(path_to_input);
    assert_not_null(path_to_expected);
    IO *input = io_create_from_file(path_to_input, "rb");
    IO *expected = io_create_from_file(path_to_expected, "rb");

    char *input_data = io_read_string(input, io_size(input));
    assert_not_null(input_data);
    VirtualFile *file = vf_create();
    assert_not_null(file);
    vf_set_data(file, input_data, io_size(input));
    free(input_data);

    converter_decode(converter, file);

    char *expected_boxed_data = io_read_string(expected, io_size(expected));
    assert_not_null(expected_boxed_data);
    Image *expected_image = image_create_from_boxed(
        expected_boxed_data,
        io_size(expected));
    assert_not_null(expected_image);
    free(expected_boxed_data);

    Image *actual_image = image_create_from_boxed(
        vf_get_data(file),
        vf_get_size(file));
    assert_not_null(actual_image);

    compare_images(expected_image, actual_image);

    vf_destroy(file);

    io_destroy(input);
    io_destroy(expected);
}
