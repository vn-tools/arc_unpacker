#include "file.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;

std::shared_ptr<File> tests::file_from_converter(
    const boost::filesystem::path &path, const fmt::Converter &converter)
{
    return converter.decode(*tests::file_from_path(path));
}

std::shared_ptr<util::Image> tests::image_from_converter(
    const boost::filesystem::path &path, const fmt::Converter &converter)
{
    return util::Image::from_boxed(file_from_converter(path, converter)->io);
}

void tests::assert_file_conversion(
    const fmt::Converter &converter,
    const std::string &input_file_path,
    const std::string &expected_file_path)
{
    auto actual = tests::file_from_converter(input_file_path, converter);
    auto expected = tests::file_from_path(expected_file_path);
    tests::compare_files(*expected, *actual, false);
}

void tests::assert_image_conversion(
    const fmt::Converter &converter,
    const std::string &input_image_path,
    const std::string &expected_image_path,
    int max_component_diff)
{
    auto actual = tests::image_from_converter(input_image_path, converter);
    auto expected = tests::image_from_path(expected_image_path);
    tests::compare_images(*expected, *actual, max_component_diff);
}
