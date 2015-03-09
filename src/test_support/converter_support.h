#ifndef TEST_SUPPORT_CONVERTER_SUPPORT_H
#define TEST_SUPPORT_CONVERTER_SUPPORT_H
#include <boost/filesystem/path.hpp>
#include <string>
#include "formats/converter.h"

void assert_decoded_image(const File &actual_file, const File &expected_file);

void assert_decoded_image(
    Converter &converter,
    const boost::filesystem::path &path_to_input,
    const boost::filesystem::path &path_to_expected);

#endif
