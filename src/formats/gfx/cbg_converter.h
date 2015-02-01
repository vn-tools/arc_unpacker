#ifndef FORMATS_GFX_CBG_CONVERTER_H
#define FORMATS_GFX_CBG_CONVERTER_H
#include "formats/converter.h"

extern const char *cbg_magic;
extern const size_t cbg_magic_length;

Converter *cbg_converter_create();

#endif
