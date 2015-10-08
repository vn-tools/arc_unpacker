#include "fmt/rpgmaker/rgs/common.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::rpgmaker;

u32 rgs::advance_key(const u32 key)
{
    return key * 7 + 3;
}

std::unique_ptr<File> rgs::read_file(
    File &arc_file, const ArchiveEntryImpl &entry)
{
    auto output_file = std::make_unique<File>();
    output_file->name = entry.name;
    arc_file.io.seek(entry.offset);

    io::BufferedIO tmp_io;
    tmp_io.write_from_io(arc_file.io, entry.size);
    tmp_io.write("\x00\x00\x00\x00"_b);
    tmp_io.seek(0);

    u32 key = entry.key;
    for (auto i : util::range(0, entry.size, 4))
    {
        u32 chunk = tmp_io.read_u32_le();
        chunk ^= key;
        key = rgs::advance_key(key);
        output_file->io.write_u32_le(chunk);
    }
    output_file->io.truncate(entry.size);
    return output_file;
}
