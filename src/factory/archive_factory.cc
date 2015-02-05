#include <assert.h>
#include <stdlib.h>
#include <string.h>
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

typedef Archive*(*ArchiveCreator)();

typedef struct
{
    const char *name;
    ArchiveCreator creator;
} FormatDefinition;

static void add_format(
    ArchiveFactory *factory,
    const char *name,
    ArchiveCreator creator)
{
    assert(factory != NULL);
    assert(name != NULL);
    assert(creator != NULL);
    FormatDefinition *definition
        = (FormatDefinition*)malloc(sizeof(FormatDefinition));
    definition->name = name;
    definition->creator = creator;
    dictionary_set(factory->formats, name, (void*)definition);
}

static void init_factory(ArchiveFactory *factory)
{
    add_format(factory, "fjsys", &fjsys_archive_create);
    add_format(factory, "arc", &arc_archive_create);
    add_format(factory, "npa", &npa_archive_create);
    add_format(factory, "xp3", &xp3_archive_create);
    add_format(factory, "rpa", &rpa_archive_create);
    add_format(factory, "pak", &pak_archive_create);
    add_format(factory, "mbl", &mbl_archive_create);
    add_format(factory, "sar", &sar_archive_create);
}



ArchiveFactory *archive_factory_create()
{
    ArchiveFactory *factory = (ArchiveFactory*)malloc(sizeof(ArchiveFactory));
    assert(factory != NULL);
    factory->formats = dictionary_create();
    init_factory(factory);
    return factory;
}

void archive_factory_destroy(ArchiveFactory *factory)
{
    size_t i;
    const Array *definitions;
    assert(factory != NULL);
    definitions = dictionary_get_values(factory->formats);
    for (i = 0; i < array_size(definitions); i ++)
        free(array_get(definitions, i));
    dictionary_destroy(factory->formats);
    free(factory);
}

const Array *archive_factory_formats(const ArchiveFactory *factory)
{
    return dictionary_get_keys(factory->formats);
}

Archive *archive_factory_from_string(
    const ArchiveFactory *factory,
    const char *format)
{
    assert(factory != NULL);
    assert(format != NULL);

    FormatDefinition *definition
        = (FormatDefinition*)dictionary_get(factory->formats, format);
    if (definition != NULL)
        return definition->creator();

    log_error("Invalid archive format: %s", format);
    return NULL;
}
