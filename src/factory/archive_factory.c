#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "assert.h"
#include "collections/dictionary.h"
#include "factory/archive_factory.h"
#include "formats/arc/sar_archive.h"
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
    FormatDefinition *definition
        = (FormatDefinition*)malloc(sizeof(FormatDefinition));
    definition->name = name;
    definition->creator = creator;
    dictionary_set(factory->formats, name, (void*)definition);
}

static void init_factory(ArchiveFactory *factory)
{
    add_format(factory, "sar", &sar_archive_create);
}



ArchiveFactory *archive_factory_create()
{
    ArchiveFactory *factory = (ArchiveFactory*)malloc(sizeof(ArchiveFactory));
    assert_not_null(factory);
    factory->formats = dictionary_create();
    init_factory(factory);
    return factory;
}

void archive_factory_destroy(ArchiveFactory *factory)
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

const Array *archive_factory_formats(const ArchiveFactory *factory)
{
    return dictionary_get_keys(factory->formats);
}

Archive *archive_factory_from_string(
    const ArchiveFactory *factory,
    const char *format)
{
    FormatDefinition *definition;

    assert_not_null(factory);
    assert_not_null(format);

    definition = (FormatDefinition*)dictionary_get(factory->formats, format);
    if (definition != NULL)
        return definition->creator();

    errno = EINVAL;
    log_error("Invalid format: %s", format);
    return NULL;
}
