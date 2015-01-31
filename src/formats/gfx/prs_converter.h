#ifndef FORMATS_GFX_PRS_CONVERTER_H
#define FORMATS_GFX_PRS_CONVERTER_H
#include "formats/converter.h"

extern const char *prs_magic;
extern const size_t prs_magic_length;

Converter *prs_converter_create();

#endif
