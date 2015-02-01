#ifndef FORMATS_GFX_XYZ_CONVERTER_H
#define FORMATS_GFX_XYZ_CONVERTER_H
#include "formats/converter.h"

extern const char *xyz_magic;
extern const size_t xyz_magic_length;

Converter *xyz_converter_create();

#endif
