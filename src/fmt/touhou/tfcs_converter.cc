// TFCS text list file
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .csv
//
// Known games:
// - Touhou 13.5 - Hopeless Masquerade

#include <boost/algorithm/string.hpp>
#include "fmt/touhou/tfcs_converter.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/zlib.h"

using namespace au;
using namespace au::fmt::touhou;

static const std::string magic("TFCS\x00", 5);

static void write_cell(io::IO &output_io, std::string cell)
{
    if (cell.find(",") != std::string::npos)
    {
        boost::replace_all(cell, "\"", "\"\"");
        cell = "\"" + cell + "\"";
    }
    output_io.write(util::sjis_to_utf8(cell));
}

bool TfcsConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> TfcsConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    size_t compressed_size = file.io.read_u32_le();
    size_t original_size = file.io.read_u32_le();
    io::BufferedIO uncompressed_io(
        util::zlib_inflate(file.io.read_until_end()));
    if (uncompressed_io.size() != original_size)
        throw std::runtime_error("Unexpected file size");

    std::unique_ptr<File> output_file(new File);
    output_file->name = file.name;
    output_file->change_extension("csv");

    size_t row_count = uncompressed_io.read_u32_le();
    for (size_t i = 0; i < row_count; i++)
    {
        size_t column_count = uncompressed_io.read_u32_le();
        for (size_t j = 0; j < column_count; j++)
        {
            size_t cell_size = uncompressed_io.read_u32_le();
            std::string cell = uncompressed_io.read(cell_size);

            //escaping etc. is too boring
            write_cell(output_file->io, cell);
            if (j != column_count - 1)
                output_file->io.write(",");
        }
        output_file->io.write("\n");
    }

    return output_file;
}
