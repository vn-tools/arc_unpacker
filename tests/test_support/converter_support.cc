#include "file.h"
#include "io/file_io.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"
#include "util/image.h"

using namespace au;

static void compare_images(
    const util::Image &expected_image, const util::Image &actual_image)
{
    REQUIRE(expected_image.width() == actual_image.width());
    REQUIRE(expected_image.height() == actual_image.height());

    size_t x, y;
    for (y = 0; y < expected_image.height(); y++)
    {
        for (x = 0; x < expected_image.width(); x++)
        {
            u32 expected_rgba = expected_image.color_at(x, y);
            u32 actual_rgba = actual_image.color_at(x, y);
            if (expected_rgba != actual_rgba) //speed up
            {
                INFO("Pixels differ at " << x << ", " << y);
                REQUIRE(expected_rgba == actual_rgba);
            }
        }
    }
}

static void compare_files(const File &expected_file, const File &actual_file)
{
    REQUIRE(expected_file.io.size() == actual_file.io.size());
    expected_file.io.seek(0);
    actual_file.io.seek(0);
    REQUIRE(expected_file.io.read_to_eof() == actual_file.io.read_to_eof());
}

static std::unique_ptr<File> file_from_path(const boost::filesystem::path &path)
{
    return std::unique_ptr<File>(new File(path, io::FileMode::Read));
}

static std::unique_ptr<File> file_from_converter(
    const boost::filesystem::path &path, fmt::Converter &converter)
{
    return converter.decode(*file_from_path(path));
}

static std::unique_ptr<util::Image> image_from_converter(
    const boost::filesystem::path &path, fmt::Converter &converter)
{
    return util::Image::from_boxed(file_from_converter(path, converter)->io);
}

static std::unique_ptr<util::Image> image_from_path(
    const boost::filesystem::path &path)
{
    return util::Image::from_boxed(file_from_path(path)->io);
}

void au::tests::assert_decoded_image(
    const File &actual_file, const File &expected_file)
{
    auto actual_image = util::Image::from_boxed(actual_file.io);
    auto expected_image = util::Image::from_boxed(expected_file.io);
    compare_images(*expected_image, *actual_image);
}

void au::tests::assert_decoded_image(
    fmt::Converter &converter,
    const boost::filesystem::path &path_to_input,
    const boost::filesystem::path &path_to_expected)
{
    auto actual_image = image_from_converter(path_to_input, converter);
    auto expected_image = image_from_path(path_to_expected);
    compare_images(*expected_image, *actual_image);
}

void au::tests::assert_decoded_file(
    fmt::Converter &converter,
    const boost::filesystem::path &path_to_input,
    const boost::filesystem::path &path_to_expected)
{
    auto actual_file = file_from_converter(path_to_input, converter);
    auto expected_file = file_from_path(path_to_expected);
    compare_files(*expected_file, *actual_file);
}
