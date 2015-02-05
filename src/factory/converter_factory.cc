#include <cassert>
#include <functional>
#include "collections/dictionary.h"
#include "factory/converter_factory.h"
#include "formats/gfx/cbg_converter.h"
#include "formats/gfx/g00_converter.h"
#include "formats/gfx/mgd_converter.h"
#include "formats/gfx/prs_converter.h"
#include "formats/gfx/xyz_converter.h"
#include "formats/sfx/nwa_converter.h"
#include "logger.h"

struct ConverterFactory
{
    Dictionary *formats;
};

typedef struct
{
    const char *name;
    std::function<Converter*()> creator;
} FormatDefinition;

static void add_format(
    ConverterFactory *factory,
    const char *name,
    std::function<Converter*()> creator)
{
    assert(factory != nullptr);
    assert(name != nullptr);
    assert(creator != nullptr);
    FormatDefinition *definition = new FormatDefinition;
    definition->name = name;
    definition->creator = creator;
    dictionary_set(factory->formats, name, (void*)definition);
}

static void init_factory(ConverterFactory *factory)
{
    assert(factory != nullptr);
    add_format(factory, "cbg", []() { return new CbgConverter(); });
    add_format(factory, "xyz", []() { return new XyzConverter(); });
    add_format(factory, "mgd", []() { return new MgdConverter(); });
    add_format(factory, "g00", []() { return new G00Converter(); });
    add_format(factory, "nwa", []() { return new NwaConverter(); });
    add_format(factory, "prs", []() { return new PrsConverter(); });
}



ConverterFactory *converter_factory_create()
{
    ConverterFactory *factory = new ConverterFactory;
    assert(factory != nullptr);
    factory->formats = dictionary_create();
    init_factory(factory);
    return factory;
}

void converter_factory_destroy(ConverterFactory *factory)
{
    size_t i;
    const Array *definitions;
    assert(factory != nullptr);
    definitions = dictionary_get_values(factory->formats);
    for (i = 0; i < array_size(definitions); i ++)
        delete (FormatDefinition*)array_get(definitions, i);
    dictionary_destroy(factory->formats);
    delete factory;
}

const Array *converter_factory_formats(const ConverterFactory *factory)
{
    return dictionary_get_keys(factory->formats);
}

Converter *converter_factory_from_string(
    const ConverterFactory *factory,
    const char *format)
{
    assert(factory != nullptr);
    assert(format != nullptr);

    FormatDefinition *definition
        = (FormatDefinition*)dictionary_get(factory->formats, format);
    if (definition != nullptr)
        return definition->creator();

    log_error("Invalid converter format: %s", format);
    return nullptr;
}
