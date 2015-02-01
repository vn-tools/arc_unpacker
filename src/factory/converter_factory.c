#include <stdlib.h>
#include "assert_ex.h"
#include "collections/dictionary.h"
#include "factory/converter_factory.h"
#include "formats/gfx/prs_converter.h"
#include "logger.h"

struct ConverterFactory
{
    Dictionary *formats;
};

typedef Converter*(*ConverterCreator)();

typedef struct
{
    const char *name;
    ConverterCreator creator;
} FormatDefinition;

static void add_format(
    ConverterFactory *factory,
    const char *name,
    ConverterCreator creator)
{
    assert_not_null(factory);
    assert_not_null(name);
    assert_that(creator != NULL);
    FormatDefinition *definition
        = (FormatDefinition*)malloc(sizeof(FormatDefinition));
    definition->name = name;
    definition->creator = creator;
    dictionary_set(factory->formats, name, (void*)definition);
}

static void init_factory(ConverterFactory *factory)
{
    assert_not_null(factory);
    add_format(factory, "prs", &prs_converter_create);
}



ConverterFactory *converter_factory_create()
{
    ConverterFactory *factory
        = (ConverterFactory*)malloc(sizeof(ConverterFactory));
    assert_not_null(factory);
    factory->formats = dictionary_create();
    init_factory(factory);
    return factory;
}

void converter_factory_destroy(ConverterFactory *factory)
{
    size_t i;
    const Array *definitions;
    assert_not_null(factory);
    definitions = dictionary_get_values(factory->formats);
    for (i = 0; i < array_size(definitions); i ++)
        free(array_get(definitions, i));
    dictionary_destroy(factory->formats);
    free(factory);
}

const Array *converter_factory_formats(const ConverterFactory *factory)
{
    return dictionary_get_keys(factory->formats);
}

Converter *converter_factory_from_string(
    const ConverterFactory *factory,
    const char *format)
{
    FormatDefinition *definition;

    assert_not_null(factory);
    assert_not_null(format);

    definition = (FormatDefinition*)dictionary_get(factory->formats, format);
    if (definition != NULL)
        return definition->creator();

    log_error("Invalid converter format: %s", format);
    return NULL;
}
