#include "fmt/touhou/tfcs_converter.h"
#include <boost/algorithm/string.hpp>
#include "err.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

static const bstr magic = "TFCS\x00"_b;

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
        util::pack::zlib_inflate(file.io.read_to_eof()));

    std::unique_ptr<File> output_file(new File);
    output_file->name = file.name;
    output_file->change_extension("csv");

    size_t row_count = uncompressed_io.read_u32_le();
    for (size_t i : util::range(row_count))
    {
        size_t column_count = uncompressed_io.read_u32_le();
        for (size_t j : util::range(column_count))
        {
            size_t cell_size = uncompressed_io.read_u32_le();
            std::string cell = uncompressed_io.read(cell_size).str();

            //escaping etc. is too boring
            write_cell(output_file->io, cell);
            if (j != column_count - 1)
                output_file->io.write(","_b);
        }
        output_file->io.write("\n"_b);
    }

    return output_file;
}

static auto dummy = fmt::Registry::add<TfcsConverter>("th/tfcs");
