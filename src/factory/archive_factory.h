#ifndef FACTORY_ARCHIVE_FACTORY_H
#define FACTORY_ARCHIVE_FACTORY_H
#include "formats/archive.h"

typedef struct ArchiveFactory ArchiveFactory;

ArchiveFactory *archive_factory_create();

void archive_factory_destroy(ArchiveFactory *factory);

const Array *archive_factory_formats(const ArchiveFactory *factory);

Archive *archive_factory_from_string(
    const ArchiveFactory *factory,
    const char *format);

#endif
