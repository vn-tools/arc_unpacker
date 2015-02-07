#include <cassert>
#include <cstring>
#include <memory>
#include "formats/image.h"
#include "file_io.h"
#include "logger.h"
#include "test_support/converter_support.h"
#include "virtual_file.h"

namespace
{
    void compare_images(
        const Image &expected_image,
        const Image &actual_image)
    {
        assert(expected_image.width() == actual_image.width());
        assert(expected_image.height() == actual_image.height());

        size_t x, y;
        for (y = 0; y < expected_image.height(); y ++)
        {
            for (x = 0; x < expected_image.width(); x ++)
            {
                uint32_t expected_rgba = expected_image.color_at(x, y);
                uint32_t actual_rgba = actual_image.color_at(x, y);
                if (expected_rgba != actual_rgba)
                {
                    log_error(
                        "Image pixels differ at %d, %d (%08x != %08x)",
                        x,
                        y,
                        expected_rgba,
                        actual_rgba);
                    assert(0);
                }
            }
        }
    }

    std::unique_ptr<Image> get_actual_image(
        const std::string path, Converter &converter)
    {
        FileIO io(path, "rb");
        VirtualFile file;
        file.io.write_from_io(io, io.size());
        converter.decode(file);
        return std::unique_ptr<Image>(Image::from_boxed(file.io));
    }

    std::unique_ptr<Image> get_expected_image(const std::string path)
    {
        FileIO io(path, "rb");
        return std::unique_ptr<Image>(Image::from_boxed(io));
    }
}

void assert_decoded_image(
    Converter &converter,
    const std::string &path_to_input,
    const std::string &path_to_expected)
{
    auto actual_image = get_actual_image(path_to_input, converter);
    auto expected_image = get_expected_image(path_to_expected);

    compare_images(*expected_image, *actual_image);
}
