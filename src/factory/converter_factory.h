#ifndef FACTORY_CONVERTER_FACTORY_H
#define FACTORY_CONVERTER_FACTORY_H
#include "formats/converter.h"
#include "collections/array.h"

typedef struct ConverterFactory ConverterFactory;

ConverterFactory *converter_factory_create();

void converter_factory_destroy(ConverterFactory *factory);

const Array *converter_factory_formats(const ConverterFactory *factory);

Converter *converter_factory_from_string(
    const ConverterFactory *factory,
    const char *format);

#endif
