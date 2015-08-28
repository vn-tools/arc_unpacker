// MGR image container
//
// Company:   Propeller
// Engine:    -
// Extension: .mgr
//
// Known games:
// - Sukimazakura to Uso no Machi

#include "fmt/propeller/mgr_archive.h"
#include "io/buffered_io.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::propeller;

static bstr decompress(const bstr &input, size_t size_original)
{
    const u8 *input_ptr = input.get<const u8>();
    const u8 *input_end = input_ptr + input.size();

    bstr output;
    output.resize(size_original);
    u8 *output_ptr = output.get<u8>();
    u8 *output_end = output_ptr + size_original;

    while (output_ptr < output_end)
    {
        u32 c = *input_ptr++;

        if (c < 0x20)
        {
            u32 size = c + 1;
            while (size--)
                *output_ptr++ = *input_ptr++;
        }
        else
        {
            u32 size = c >> 5;
            if (size == 7)
                size += *input_ptr++;
            size += 2;

            u32 look_behind = ((c & 0x1F) << 8) + 1;
            look_behind += *input_ptr++;

            u8 *source = output_ptr - look_behind;
            while (size--)
                *output_ptr++ = *source++;
        }
    }

    return output;
}

bool MgrArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("mgr");
}

void MgrArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    size_t entry_count = arc_file.io.read_u16_le();

    std::vector<size_t> offsets;
    offsets.reserve(entry_count);

    if (entry_count == 1)
    {
        offsets.push_back(arc_file.io.tell());
    }
    else
    {
        for (auto i : util::range(entry_count))
            offsets.push_back(arc_file.io.read_u32_le());
    }

    size_t file_number = 0;
    for (auto &offset : offsets)
    {
        arc_file.io.seek(offset);
        size_t size_original = arc_file.io.read_u32_le();
        size_t size_compressed = arc_file.io.read_u32_le();

        std::unique_ptr<File> file(new File);
        file->name = util::format("%d.bmp", ++file_number);

        auto data = arc_file.io.read(size_compressed);
        data = decompress(data, size_original);
        file->io.write(data);

        file_saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<MgrArchive>("propeller/mgr");
