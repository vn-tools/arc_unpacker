#include <cassert>
#include <cstring>
#include <functional>
#include "collections/dictionary.h"
#include "factory/archive_factory.h"
#include "formats/arc/arc_archive.h"
#include "formats/arc/fjsys_archive.h"
#include "formats/arc/mbl_archive.h"
#include "formats/arc/npa_archive.h"
#include "formats/arc/pak_archive.h"
#include "formats/arc/rpa_archive.h"
#include "formats/arc/sar_archive.h"
#include "formats/arc/xp3_archive.h"
#include "logger.h"

struct ArchiveFactory
{
    Dictionary *formats;
};

typedef struct
{
    const char *name;
    std::function<Archive*()> creator;
} FormatDefinition;

static void add_format(
    ArchiveFactory *factory,
    const char *name,
    std::function<Archive*()> creator)
{
    assert(factory != nullptr);
    assert(name != nullptr);
    assert(creator != nullptr);
    FormatDefinition *definition = new FormatDefinition;
    definition->name = name;
    definition->creator = creator;
    dictionary_set(factory->formats, name, (void*)definition);
}

static void init_factory(ArchiveFactory *factory)
{
    add_format(factory, "fjsys", []() { return new FjsysArchive(); });
    add_format(factory, "arc", []() { return new ArcArchive(); });
    add_format(factory, "npa", []() { return new NpaArchive(); });
    add_format(factory, "xp3", []() { return new Xp3Archive(); });
    add_format(factory, "rpa", []() { return new RpaArchive(); });
    add_format(factory, "pak", []() { return new PakArchive(); });
    add_format(factory, "mbl", []() { return new MblArchive(); });
    add_format(factory, "sar", []() { return new SarArchive(); });
}



ArchiveFactory *archive_factory_create()
{
    ArchiveFactory *factory = new ArchiveFactory;
    assert(factory != nullptr);
    factory->formats = dictionary_create();
    init_factory(factory);
    return factory;
}

void archive_factory_destroy(ArchiveFactory *factory)
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

const Array *archive_factory_formats(const ArchiveFactory *factory)
{
    return dictionary_get_keys(factory->formats);
}

Archive *archive_factory_from_string(
    const ArchiveFactory *factory,
    const char *format)
{
    assert(factory != nullptr);
    assert(format != nullptr);

    FormatDefinition *definition
        = (FormatDefinition*)dictionary_get(factory->formats, format);
    if (definition != nullptr)
        return definition->creator();

    log_error("Invalid archive format: %s", format);
    return nullptr;
}
