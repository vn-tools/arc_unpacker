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

static void decompress(
    io::BufferedIO &input_io, io::IO &output_io, size_t size_original)
{
    u8 *input_ptr = reinterpret_cast<u8*>(input_io.buffer());
    u8 *input_guardian = input_ptr + input_io.size();

    std::unique_ptr<u8[]> output(new u8[size_original]);
    u8 *output_ptr = output.get();
    u8 *output_guardian = output.get() + size_original;

    while (output_ptr < output_guardian)
    {
        u32 c = *input_ptr++;

        if (c < 0x20)
        {
            u32 length = c + 1;
            while (length--)
                *output_ptr++ = *input_ptr++;
        }
        else
        {
            u32 length = c >> 5;
            if (length == 7)
                length += *input_ptr++;
            length += 2;

            u32 look_behind = ((c & 0x1F) << 8) + 1;
            look_behind += *input_ptr++;

            u8 *source = output_ptr - look_behind;
            while (length--)
                *output_ptr++ = *source++;
        }
    }

    output_io.write(output.get(), size_original);
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

        io::BufferedIO input(arc_file.io, size_compressed);
        decompress(input, file->io, size_original);

        file_saver.save(std::move(file));
    }
}
