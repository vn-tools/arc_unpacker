#include "file.h"
#include "io/file_io.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"
#include "util/format.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;

static void compare_images(
    const util::Image &expected_image, const util::Image &actual_image)
{
    REQUIRE(expected_image.pixels().width() == actual_image.pixels().width());
    REQUIRE(expected_image.pixels().height() == actual_image.pixels().height());

    for (auto y : util::range(expected_image.pixels().height()))
    for (auto x : util::range(expected_image.pixels().width()))
    {
        auto expected_pix = expected_image.pixels().at(x, y);
        auto actual_pix = actual_image.pixels().at(x, y);
        if (expected_pix != actual_pix) //speed up
        {
            INFO(util::format(
                "Pixels differ at %d, %d: %02x%02x%02x%02x != %02x%02x%02x%02x",
                x, y,
                expected_pix.b, expected_pix.g, expected_pix.r, expected_pix.a,
                actual_pix.b, actual_pix.g, actual_pix.r, actual_pix.a));
            REQUIRE(expected_pix == actual_pix);
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
