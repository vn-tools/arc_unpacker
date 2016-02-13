#include "dec/rpgmaker/rgs/common.h"
#include "algo/range.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::rpgmaker;

u32 rgs::advance_key(const u32 key)
{
    return key * 7 + 3;
}

std::unique_ptr<io::File> rgs::read_file_impl(
    io::File &arc_file, const CustomArchiveEntry &entry)
{
    auto output_file = std::make_unique<io::File>();
    output_file->path = entry.path;
    arc_file.stream.seek(entry.offset);

    io::MemoryStream tmp_stream;
    tmp_stream.write(arc_file.stream.read(entry.size));
    tmp_stream.write("\x00\x00\x00\x00"_b);
    tmp_stream.seek(0);

    u32 key = entry.key;
    for (const auto i : algo::range(0, entry.size, 4))
    {
        const auto chunk = tmp_stream.read_le<u32>() ^ key;
        key = rgs::advance_key(key);
        output_file->stream.write_le<u32>(chunk);
    }
    output_file->stream.resize(entry.size);
    return output_file;
}
