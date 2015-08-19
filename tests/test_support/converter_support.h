#ifndef AU_TEST_SUPPORT_CONVERTER_SUPPORT_H
#define AU_TEST_SUPPORT_CONVERTER_SUPPORT_H
#include <boost/filesystem/path.hpp>
#include "file.h"
#include "fmt/converter.h"
#include "util/image.h"

namespace au {
namespace tests {

    std::shared_ptr<File> file_from_converter(
        const boost::filesystem::path &path, const fmt::Converter &converter);

    std::shared_ptr<util::Image> image_from_converter(
        const boost::filesystem::path &path, const fmt::Converter &converter);

    void assert_file_conversion(
        const fmt::Converter &converter,
        const std::string &input_image_path,
        const std::string &expected_image_path);

    void assert_image_conversion(
        const fmt::Converter &converter,
        const std::string &input_image_path,
        const std::string &expected_image_path,
        int max_component_diff = 0);

} }

#endif
