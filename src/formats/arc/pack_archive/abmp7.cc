#include "formats/arc/pack_archive/abmp7.h"
#include "string_ex.h"

namespace
{
    const std::string magic("ABMP7", 5);

    void read_first_file(
        std::vector<std::unique_ptr<File>> &files,
        File &b_file)
    {
        size_t length = b_file.io.read_u32_le();
        std::unique_ptr<File> subfile(new File);
        subfile->io.write_from_io(b_file.io, length);
        subfile->name = b_file.name + "$.dat";
        subfile->guess_extension();
        files.push_back(std::move(subfile));
    }

    void read_next_file(
        std::vector<std::unique_ptr<File>> &files,
        File &b_file)
    {
        std::string encoded_name = b_file.io.read(b_file.io.read_u8());
        b_file.io.skip(31 - encoded_name.size());
        std::string name = convert_encoding(encoded_name, "cp932", "utf-8");
        size_t length = b_file.io.read_u32_le();
        std::unique_ptr<File> subfile(new File);
        subfile->io.write_from_io(b_file.io, length);
        subfile->name = b_file.name + "_" + name + ".dat";
        subfile->guess_extension();
        files.push_back(std::move(subfile));
    }
}

bool abmp7_unpack(std::vector<std::unique_ptr<File>> &files, File &b_file)
{
    b_file.io.seek(0);
    if (b_file.io.size() < 12)
        return false;
    if (b_file.io.read(magic.size()) != magic)
        return false;

    b_file.io.seek(12);
    b_file.io.skip(b_file.io.read_u32_le());

    read_first_file(files, b_file);
    while (b_file.io.tell() < b_file.io.size())
        read_next_file(files, b_file);
    return true;
}
