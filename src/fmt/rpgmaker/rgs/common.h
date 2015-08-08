#ifndef AU_FMT_RPGMAKER_RGS_COMMON_H
#define AU_FMT_RPGMAKER_RGS_COMMON_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace rpgmaker {
namespace rgs {

    struct TableEntry
    {
        std::string name;
        size_t size;
        size_t offset;
        u32 key;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;

    u32 advance_key(const u32 key);
    std::unique_ptr<File> read_file(io::IO &arc_io, TableEntry &entry);

} } } }

#endif
