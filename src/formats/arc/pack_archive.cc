// PACK archive
//
// Company:   -
// Engine:    QLiE
// Extension: .pack
//
// Known games:
// - Soshite Ashita no Sekai yori

#include "buffered_io.h"
#include "formats/arc/pack_archive.h"
#include "string_ex.h"

namespace
{
    void guess_extension(VirtualFile &file)
    {
        std::vector<std::pair<std::string, std::string>> definitions
        {
            { "b", std::string("abmp", 4) }, // why
            { "png", std::string("\x89PNG", 4) },
            { "bmp", std::string("BM", 2) },
            { "wav", std::string("RIFF", 4) },
            { "ogg", std::string("OggS", 4) },
            { "imoavi", std::string("IMOAVI", 6) },
            { "jpeg", std::string("\xff\xd8\xff", 3) },
        };

        for (auto &def : definitions)
        {
            const std::string ext = def.first;
            const std::string magic = def.second;
            file.io.seek(0);
            if (file.io.size() < magic.size()) continue;
            if (file.io.read(magic.size()) != magic) continue;
            file.change_extension(ext);
            return;
        }
    }

    void try_unpack(
        std::vector<std::unique_ptr<VirtualFile>> &files,
        std::unique_ptr<VirtualFile> file);

    namespace B7
    {
        const std::string magic("ABMP7", 5);

        void read_first_file(
            std::vector<std::unique_ptr<VirtualFile>> &files,
            VirtualFile &b_file)
        {
            size_t length = b_file.io.read_u32_le();
            std::unique_ptr<VirtualFile> subfile(new VirtualFile);
            subfile->io.write_from_io(b_file.io, length);
            subfile->name = b_file.name + "$.dat";
            guess_extension(*subfile);
            files.push_back(std::move(subfile));
        }

        void read_next_file(
            std::vector<std::unique_ptr<VirtualFile>> &files,
            VirtualFile &b_file)
        {
            std::string encoded_name = b_file.io.read(b_file.io.read_u8());
            b_file.io.skip(31 - encoded_name.size());
            std::string name = convert_encoding(encoded_name, "cp932", "utf-8");
            size_t length = b_file.io.read_u32_le();
            std::unique_ptr<VirtualFile> subfile(new VirtualFile);
            subfile->io.write_from_io(b_file.io, length);
            subfile->name = b_file.name + "_" + name + ".dat";
            guess_extension(*subfile);
            files.push_back(std::move(subfile));
        }

        bool unpack(
            std::vector<std::unique_ptr<VirtualFile>> &files,
            VirtualFile &b_file)
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
    }

    namespace B10
    {
        // This must be the worst designed format I've worked with
        const std::string magic10("abmp10\0\0\0\0\0\0\0\0\0\0", 16);
        const std::string magic11("abmp11\0\0\0\0\0\0\0\0\0\0", 16);
        const std::string magic12("abmp12\0\0\0\0\0\0\0\0\0\0", 16);
        const std::string magic_imgdat10("abimgdat10\0\0\0\0\0\0", 16);
        const std::string magic_imgdat11("abimgdat11\0\0\0\0\0\0", 16);
        const std::string magic_imgdat13("abimgdat13\0\0\0\0\0\0", 16);
        const std::string magic_imgdat14("abimgdat14\0\0\0\0\0\0", 16);
        const std::string magic_snddat10("absnddat10\0\0\0\0\0\0", 16);
        const std::string magic_snddat11("absnddat11\0\0\0\0\0\0", 16);
        const std::string magic_data10("abdata10\0\0\0\0\0\0\0\0", 16);
        const std::string magic_data11("abdata11\0\0\0\0\0\0\0\0", 16);
        const std::string magic_data12("abdata12\0\0\0\0\0\0\0\0", 16);
        const std::string magic_data13("abdata13\0\0\0\0\0\0\0\0", 16);
        const std::string magic_image10("abimage10\0\0\0\0\0\0\0", 16);
        const std::string magic_sound10("absound10\0\0\0\0\0\0\0", 16);

        int get_version(IO &b_io)
        {
            if (b_io.size() < 16)
                return 0;

            std::string magic = b_io.read(16);
            if (magic == magic10)
                return 10;
            if (magic == magic11)
                return 11;
            if (magic == magic12)
                return 12;
            return 0;
        }

        std::unique_ptr<VirtualFile> read_subfile(
            IO &b_io, const std::string base_name)
        {
            std::string magic = b_io.read(16);
            std::string encoded_name = b_io.read(b_io.read_u16_le());
            std::string name = convert_encoding(encoded_name, "cp932", "utf-8");

            if (magic == magic_snddat11
            || magic == magic_imgdat11
            || magic == magic_imgdat13
            || magic == magic_imgdat14)
            {
                b_io.skip(b_io.read_u16_le());
            }
            else if (magic != magic_imgdat10 && magic != magic_snddat10)
                throw std::runtime_error("Unknown image magic " + magic);

            b_io.skip(1);

            if (magic == magic_imgdat14)
                b_io.skip(76);
            else if (magic == magic_imgdat13)
                b_io.skip(12);

            size_t len = b_io.read_u32_le();
            if (len == 0)
                return nullptr;

            std::unique_ptr<VirtualFile> subfile(new VirtualFile);
            subfile->io.write_from_io(b_io, len);
            subfile->name = base_name + "_" + name + ".dat";
            guess_extension(*subfile);
            return subfile;
        }

        bool unpack(
            std::vector<std::unique_ptr<VirtualFile>> &files,
            VirtualFile &b_file)
        {
            b_file.io.seek(0);
            int version = get_version(b_file.io);
            if (!version)
                return false;

            while (b_file.io.tell() < b_file.io.size())
            {
                std::string magic = b_file.io.read(16);
                if (magic == magic_data10
                || magic == magic_data11
                || magic == magic_data12
                || magic == magic_data13)
                {
                    //interesting
                    size_t size = b_file.io.read_u32_le();
                    b_file.io.skip(size);
                }
                else if (magic == magic_image10)
                {
                    size_t image_count = b_file.io.read_u8();
                    for (size_t i = 0; i < image_count; i ++)
                    {
                        auto subfile = read_subfile(
                            b_file.io, b_file.name + "_" + stoi(i));
                        try_unpack(files, std::move(subfile));
                    }
                }
                else if (magic == magic_sound10)
                {
                    size_t sound_count = b_file.io.read_u8();
                    for (size_t i = 0; i < sound_count; i ++)
                    {
                        auto subfile = read_subfile(
                            b_file.io, b_file.name + "_" + stoi(i));
                        try_unpack(files, std::move(subfile));
                    }
                }
                else
                    throw std::runtime_error("Unknown section " + magic);
            }
            return true;
        }
    }

    void try_unpack(
        std::vector<std::unique_ptr<VirtualFile>> &files,
        std::unique_ptr<VirtualFile> file)
    {
        if (file == nullptr)
            return;

        try
        {
            if (!B7::unpack(files, *file) && !B10::unpack(files, *file))
                files.push_back(std::move(file));
        }
        catch (std::exception &e)
        {
            files.push_back(std::move(file));
        }
    }

    namespace QLiE
    {
        const std::string magic1("FilePackVer1.0\x00\x00", 16);
        const std::string magic2("FilePackVer2.0\x00\x00", 16);
        const std::string magic3("FilePackVer3.0\x00\x00", 16);

        typedef struct
        {
            std::string name;
            size_t size_compressed;
            size_t size_original;
            size_t offset;
            bool encrypted;
            bool compressed;
            uint32_t key;
        } TableEntry;

        typedef std::vector<std::unique_ptr<TableEntry>> Table;

        int get_version(IO &arc_io)
        {
            int version = 1;
            for (auto &magic : {magic1, magic2, magic3})
            {
                arc_io.seek(arc_io.size() - 0x1c);
                if (arc_io.read(magic.size()) == magic)
                    return version;
                ++ version;
            }
            return 0;
        }

        namespace v3
        {
            uint64_t padw(uint64_t a, uint64_t b)
            {
                return ((a & 0x7FFF7FFF7FFF7FFF)
                    + (b & 0x7FFF7FFF7FFF7FFF))
                    ^ ((a ^ b) & 0x8000800080008000);
            }

            uint64_t padd(uint64_t a, uint64_t b)
            {
                return ((a & 0x7FFFFFFF7FFFFFFF)
                    + (b & 0x7FFFFFFF7FFFFFFF))
                    ^ ((a ^ b) & 0x8000000080000000);
            }

            uint32_t derive_key(IO &io, size_t bytes)
            {
                uint64_t key = 0;
                uint64_t result = 0;
                size_t size = bytes / 8;
                std::unique_ptr<uint64_t[]> input(new uint64_t[size]);
                uint64_t *input_ptr = input.get();
                uint64_t *input_guardian = input_ptr + size;
                io.read(input_ptr, bytes);

                while (input_ptr < input_guardian)
                {
                    key = padw(key, 0x0307030703070307);
                    result = padw(result, *input_ptr++ ^ key);
                }
                result ^= (result >> 32);
                return static_cast<uint32_t>(result & 0xFFFFFFFF);
            }

            void decrypt_file_name(
                unsigned char *file_name, size_t length, uint32_t key)
            {
                unsigned char _xor = ((key ^ 0x3e) + length) & 0xff;
                for (size_t i = 1; i <= length; i ++)
                {
                    file_name[i - 1] ^= ((i ^ _xor) & 0xff) + i;
                }
            }

            void decrypt_file_data(
                unsigned char *file_data, size_t length, uint32_t seed)
            {
                uint64_t key = 0xA73C5F9DA73C5F9D;
                uint64_t mutator = (seed + length) ^ 0xFEC9753E;
                mutator = (mutator << 32) | mutator;

                uint64_t *input_ptr = reinterpret_cast<uint64_t*>(file_data);
                uint64_t *input_guardian = input_ptr + (length / 8);

                while (input_ptr < input_guardian)
                {
                    key = padd(key, 0xCE24F523CE24F523);
                    key ^= mutator;
                    mutator = *input_ptr++ ^= key;
                }
            }
        }

        void decompress(
            const char *input,
            size_t input_size,
            char *output,
            size_t output_size)
        {
            char *output_ptr = output;
            const char *output_guardian = output + output_size;

            BufferedIO input_io(input, input_size);

            std::string magic("1PC\xff", 4);
            if (input_io.read(magic.length()) != magic)
                throw std::runtime_error("Unexpected magic");

            bool use_short_length = input_io.read_u32_le() > 0;
            uint32_t file_size = input_io.read_u32_le();
            if (file_size != output_size)
                throw std::runtime_error("Unpexpected file size");

            unsigned char dict1[256];
            unsigned char dict2[256];
            unsigned char dict3[256];
            while (input_io.tell() < input_io.size())
            {
                for (size_t i = 0; i < 256; i ++)
                    dict1[i] = i;

                for (size_t d = 0; d < 256; )
                {
                    unsigned char c = input_io.read_u8();
                    if (c > 0x7F)
                    {
                        d += c - 0x7F;
                        c = 0;
                        if (d >= 256)
                            break;
                    }

                    for (size_t i = 0; i <= c; i ++)
                    {
                        dict1[d] = input_io.read_u8();
                        if (dict1[d] != d)
                            dict2[d] = input_io.read_u8();
                        d++;
                    }
                }

                int bytes_left = use_short_length
                    ? input_io.read_u16_le()
                    : input_io.read_u32_le();

                unsigned char n = 0;
                while (true)
                {
                    unsigned char d;
                    if (n > 0)
                    {
                        d = dict3[--n];
                    }
                    else
                    {
                        if (bytes_left == 0)
                            break;
                        bytes_left--;
                        d = input_io.read_u8();
                    }

                    if (dict1[d] == d)
                    {
                        *output_ptr++ = d;
                        if (output_ptr >= output_guardian)
                            return;
                    }
                    else
                    {
                        dict3[n++] = dict2[d];
                        dict3[n++] = dict1[d];
                    }
                }
            }
        }

        std::string read_file_name(IO &table_io, uint32_t key, int version)
        {
            size_t file_name_length = table_io.read_u16_le();
            std::unique_ptr<char[]> file_name(new char[file_name_length + 1]);
            table_io.read(file_name.get(), file_name_length);
            if (version == 3)
            {
                v3::decrypt_file_name(
                    reinterpret_cast<unsigned char*>(file_name.get()),
                    file_name_length,
                    key);
            }
            else
                throw std::runtime_error("Not implemented");

            return convert_encoding(
                std::string(file_name.get(), file_name_length),
                "cp932",
                "utf-8");
        }

        Table read_table(IO &arc_io, int version)
        {
            size_t file_count = arc_io.read_u32_le();
            uint64_t table_offset = arc_io.read_u64_le();
            size_t table_size = (arc_io.size() - 0x1c) - table_offset;
            arc_io.seek(table_offset);

            BufferedIO table_io;
            table_io.write_from_io(arc_io, table_size);

            uint32_t key = 0;
            if (version == 3)
            {
                table_io.seek(0);
                for (size_t i = 0; i < file_count; i ++)
                {
                    size_t file_name_length = table_io.read_u16_le();
                    table_io.skip(file_name_length + 28);
                }
                table_io.skip(28);
                table_io.skip(table_io.read_u32_le());
                table_io.skip(36);
                key = v3::derive_key(table_io, 256) & 0x0FFFFFFF;
            }

            Table table;
            table_io.seek(0);
            for (size_t i = 0; i < file_count; i ++)
            {
                std::unique_ptr<TableEntry> table_entry(new TableEntry);

                table_entry->name = read_file_name(table_io, key, version);
                table_entry->offset = table_io.read_u64_le();
                table_entry->size_compressed = table_io.read_u32_le();
                table_entry->size_original = table_io.read_u32_le();
                table_entry->compressed = table_io.read_u32_le() > 0;
                table_entry->encrypted = table_io.read_u32_le() > 0;

                if (version == 1 /* || version == 2? */)
                {
                    table_entry->key = table_io.read_u32_le();
                }
                else
                {
                    table_entry->key = key;
                    table_io.skip(4);
                }

                table.push_back(std::move(table_entry));
            }
            return table;
        }

        std::vector<std::unique_ptr<VirtualFile>> read_file(
            IO &arc_io, const TableEntry &table_entry, int version)
        {
            std::unique_ptr<VirtualFile> file(new VirtualFile);
            file->name = table_entry.name;

            arc_io.seek(table_entry.offset);
            std::unique_ptr<char[]> data(new char[table_entry.size_compressed]);
            arc_io.read(data.get(), table_entry.size_compressed);

            if (table_entry.encrypted)
            {
                if (version == 3)
                {
                    v3::decrypt_file_data(
                        reinterpret_cast<unsigned char*>(data.get()),
                        table_entry.size_compressed,
                        table_entry.key);
                }
                else
                    throw std::runtime_error("Not implemented");
            }

            if (table_entry.compressed)
            {
                std::unique_ptr<char[]> decompressed(
                    new char[table_entry.size_original]);

                decompress(
                    data.get(),
                    table_entry.size_compressed,
                    decompressed.get(),
                    table_entry.size_original);

                data = std::move(decompressed);
            }

            file->io.write(data.get(), table_entry.size_original);

            std::vector<std::unique_ptr<VirtualFile>> files;
            try_unpack(files, std::move(file));
            return files;
        }
    }
}

void PackArchive::unpack_internal(
    VirtualFile &file, OutputFiles &output_files) const
{
    size_t version = QLiE::get_version(file.io);
    if (!version)
        throw std::runtime_error("Not a PAC archive");

    if (version == 1 || version == 2)
        throw std::runtime_error("Version 1 and 2 are not supported.");

    QLiE::Table table = QLiE::read_table(file.io, version);
    for (auto &table_entry : table)
    {
        output_files.save([&]()
        {
            return QLiE::read_file(
                file.io,
                *table_entry,
                version);
        });
    }
}
