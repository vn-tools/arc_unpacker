#ifndef TEST_SUPPORT_CONVERTER_SUPPORT_H
#define TEST_SUPPORT_CONVERTER_SUPPORT_H
#include <string>
#include "formats/converter.h"

void assert_decoded_image(
    Converter &converter,
    const std::string &path_to_input,
    const std::string &path_to_expected);

#endif
