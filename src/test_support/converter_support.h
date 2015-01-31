#ifndef TEST_SUPPORT_CONVERTER_SUPPORT_H
#define TEST_SUPPORT_CONVERTER_SUPPORT_H
#include "formats/converter.h"

void assert_decoded_image(
    Converter *converter,
    const char *path_to_input,
    const char *path_to_expected);

#endif
