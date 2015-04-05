#include "file.h"
#include "io/file_io.h"
#include "test_support/converter_support.h"
#include "test_support/eassert.h"
#include "util/image.h"

namespace
{
    void compare_images(const Image &expected_image, const Image &actual_image)
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

    void compare_files(const File &expected_file, const File &actual_file)
    {
        //eassert(expected_file.name == actual_file.name);
        eassert(expected_file.io.size() == actual_file.io.size());
        expected_file.io.seek(0);
        actual_file.io.seek(0);
        while (!expected_file.io.eof())
            eassert(expected_file.io.read_u8() == actual_file.io.read_u8());
    }

    std::unique_ptr<File> file_from_path(const boost::filesystem::path &path)
    {
        return std::unique_ptr<File>(new File(path, FileIOMode::Read));
    }

    std::unique_ptr<File> file_from_converter(
        const boost::filesystem::path &path, Converter &converter)
    {
        return converter.decode(*file_from_path(path));
    }

    std::unique_ptr<Image> image_from_converter(
        const boost::filesystem::path &path, Converter &converter)
    {
        return Image::from_boxed(file_from_converter(path, converter)->io);
    }

    std::unique_ptr<Image> image_from_path(const boost::filesystem::path &path)
    {
        return Image::from_boxed(file_from_path(path)->io);
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
    const boost::filesystem::path &path_to_input,
    const boost::filesystem::path &path_to_expected)
{
    auto actual_image = image_from_converter(path_to_input, converter);
    auto expected_image = image_from_path(path_to_expected);
    compare_images(*expected_image, *actual_image);
}

void assert_decoded_file(
    Converter &converter,
    const boost::filesystem::path &path_to_input,
    const boost::filesystem::path &path_to_expected)
{
    auto actual_file = file_from_converter(path_to_input, converter);
    auto expected_file = file_from_path(path_to_expected);
    compare_files(*expected_file, *actual_file);
}
