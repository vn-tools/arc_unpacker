#include <cassert>
#include <cstring>
#include <memory>
#include "file_io.h"
#include "formats/image.h"
#include "test_support/converter_support.h"
#include "test_support/eassert.h"
#include "file.h"

namespace
{
    void compare_images(
        const Image &expected_image,
        const Image &actual_image)
    {
        eassert(expected_image.width() == actual_image.width());
        eassert(expected_image.height() == actual_image.height());

        size_t x, y;
        for (y = 0; y < expected_image.height(); y ++)
        {
            for (x = 0; x < expected_image.width(); x ++)
            {
                uint32_t expected_rgba = expected_image.color_at(x, y);
                uint32_t actual_rgba = actual_image.color_at(x, y);
                eassert(expected_rgba == actual_rgba);
            }
        }
    }

    std::unique_ptr<Image> image_from_converter(
        const std::string path, Converter &converter)
    {
        FileIO io(path, "rb");
        File file;
        file.io.write_from_io(io, io.size());
        converter.decode(file);
        return std::unique_ptr<Image>(Image::from_boxed(file.io));
    }

    std::unique_ptr<Image> image_from_path(const std::string path)
    {
        FileIO io(path, "rb");
        return std::unique_ptr<Image>(Image::from_boxed(io));
    }
}

void assert_decoded_image(const File &actual_file, const File &expected_file)
{
    auto actual_image = Image::from_boxed(actual_file.io);
    auto expected_image = Image::from_boxed(expected_file.io);

    compare_images(*expected_image, *actual_image);
}

void assert_decoded_image(
    Converter &converter,
    const std::string &path_to_input,
    const std::string &path_to_expected)
{
    auto actual_image = image_from_converter(path_to_input, converter);
    auto expected_image = image_from_path(path_to_expected);

    compare_images(*expected_image, *actual_image);
}
