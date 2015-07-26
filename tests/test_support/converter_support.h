#ifndef AU_TEST_SUPPORT_CONVERTER_SUPPORT_H
#define AU_TEST_SUPPORT_CONVERTER_SUPPORT_H
#include <boost/filesystem/path.hpp>
#include <string>
#include "formats/converter.h"

namespace au {
namespace tests {

    void assert_decoded_image(
        const File &actual_file, const File &expected_file);

    void assert_decoded_image(
        fmt::Converter &converter,
        const boost::filesystem::path &path_to_input,
        const boost::filesystem::path &path_to_expected);

    void assert_decoded_file(
        fmt::Converter &converter,
        const boost::filesystem::path &path_to_input,
        const boost::filesystem::path &path_to_expected);

} }

#endif
