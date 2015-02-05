#ifndef FACTORY_ARCHIVE_FACTORY_H
#define FACTORY_ARCHIVE_FACTORY_H
#include <vector>
#include "formats/archive.h"

class ArchiveFactory final
{
public:
    ArchiveFactory();
    ~ArchiveFactory();
    const std::vector<std::string> get_formats() const;
    Archive *create_archive(std::string format) const;
private:
    struct Internals;
    Internals *internals;
};

#endif
