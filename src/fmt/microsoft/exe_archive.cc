// EXE executable file
//
// Company:   Microsoft
// Engine:    Microsoft Windows
// Extension: .exe
//
// Known games:
// - [LizSoft] [080910] Fortune Summoners: Secret Of The Elemental Stone (SOTES)

#include "err.h"
#include "fmt/microsoft/exe_archive.h"
#include "log.h"
#include "util/encoding.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::microsoft;

namespace
{
    struct DosHeader
    {
        bstr magic;
        u16 e_cblp;
        u16 e_cp;
        u16 e_crlc;
        u16 e_cparhdr;
        u16 e_minalloc;
        u16 e_maxalloc;
        u16 e_ss;
        u16 e_sp;
        u16 e_csum;
        u16 e_ip;
        u16 e_cs;
        u16 e_lfarlc;
        u16 e_ovno;
        u16 e_oemid;
        u16 e_oeminfo;
        u32 e_lfanew;

        DosHeader(io::IO &io);
    };

    struct ImageOptionalHeader
    {
        u16 magic;
        u8 major_linker_version;
        u8 minor_linker_version;
        u32 size_of_code;
        u32 size_of_initialized_data;
        u32 size_of_uninitialized_data;
        u32 address_of_entry_point;
        u32 base_of_code;
        u32 base_of_data;
        u32 image_base;
        u32 section_alignment;
        u32 file_alignment;
        u16 major_operating_system_version;
        u16 minor_operating_system_version;
        u16 major_image_version;
        u16 minor_image_version;
        u16 major_subsystem_version;
        u16 minor_subsystem_version;
        u32 win32_version_value;
        u32 size_of_image;
        u32 size_of_headers;
        u32 checksum;
        u16 subsystem;
        u16 dll_characteristics;
        u64 size_of_stack_reserve;
        u64 size_of_stack_commit;
        u64 size_of_heap_reserve;
        u64 size_of_heap_commit;
        u32 loader_flags;
        u32 number_of_rva_and_sizes;

        ImageOptionalHeader(io::IO &io);
    };

    struct ImageFileHeader
    {
        u16 machine;
        u16 number_of_sections;
        u32 timestamp;
        u32 pointer_to_symbol_table;
        u32 number_of_symbols;
        u16 size_of_optional_header;
        u16 characteristics;
        ImageFileHeader(io::IO &io);
    };

    struct ImageNtHeader
    {
        u32 signature;
        ImageFileHeader file_header;
        ImageOptionalHeader optional_header;
        ImageNtHeader(io::IO &io);
    };

    struct ImageDataDir
    {
        u32 virtual_address;
        u32 size;
        ImageDataDir(io::IO &io);
    };

    struct ImageSectionHeader
    {
        std::string name;
        u32 virtual_size;
        u32 physical_address;
        u32 virtual_address;
        u32 size_of_raw_data;
        u32 pointer_to_raw_data;
        u32 pointer_to_relocations;
        u32 pointer_to_line_numbers;
        u16 number_of_relocations;
        u16 number_of_line_numbers;
        u32 characteristics;
        ImageSectionHeader(io::IO &io);
    };

    struct ImageResourceDir
    {
        u32 characteristics;
        u32 timestamp;
        u16 major_version;
        u16 minor_version;
        u16 number_of_named_entries;
        u16 number_of_id_entries;
        ImageResourceDir(io::IO &io);
    };

    struct ImageResourceDirEntry
    {
        u32 offset_to_data;
        bool name_is_string;
        u32 name_offset;
        u32 name;
        u32 id;
        u32 data_is_dir;
        ImageResourceDirEntry(io::IO &io);
    };

    struct ImageResourceDataEntry
    {
        u32 offset_to_data;
        u32 size;
        u32 code_page;
        ImageResourceDataEntry(io::IO &io);
    };

    class RvaHelper
    {
    public:
        RvaHelper(
            u32 file_alignment,
            u32 section_alignment,
            const std::vector<ImageSectionHeader> &sections);

        u32 rva_to_offset(u32 rva) const;

    private:
        const ImageSectionHeader &section_for_rva(u32 rva) const;
        u32 adjust_file_alignment(u32 offset) const;
        u32 adjust_section_alignment(u32 offset) const;

        u32 file_alignment;
        u32 section_alignment;
        const std::vector<ImageSectionHeader> &sections;
    };

    struct ResourceCrawlerArgs
    {
        ResourceCrawlerArgs(io::IO &, FileSaver &, const RvaHelper &, size_t);

        io::IO &io;
        FileSaver &saver;
        const RvaHelper &rva_helper;
        size_t base_offset;
    };

    class ResourceCrawler
    {
    public:
        static void crawl(const ResourceCrawlerArgs &args);

    private:
        ResourceCrawler(const ResourceCrawlerArgs &args);
        void process_entry(size_t offset, const std::string &path);
        void process_dir(size_t offset, const std::string path = "");
        std::string read_entry_name(const ImageResourceDirEntry &entry);

        const ResourceCrawlerArgs &args;
    };
}

//keep flat hierarchy for unpacked files
static const std::string path_sep = "／";

DosHeader::DosHeader(io::IO &io)
{
    magic      = io.read(2);
    e_cblp     = io.read_u16_le();
    e_cp       = io.read_u16_le();
    e_crlc     = io.read_u16_le();
    e_cparhdr  = io.read_u16_le();
    e_minalloc = io.read_u16_le();
    e_maxalloc = io.read_u16_le();
    e_ss       = io.read_u16_le();
    e_sp       = io.read_u16_le();
    e_csum     = io.read_u16_le();
    e_ip       = io.read_u16_le();
    e_cs       = io.read_u16_le();
    e_lfarlc   = io.read_u16_le();
    e_ovno     = io.read_u16_le();
    io.skip(2 * 4);
    e_oemid    = io.read_u16_le();
    e_oeminfo  = io.read_u16_le();
    io.skip(2 * 10);
    e_lfanew   = io.read_u32_le();
}

ImageOptionalHeader::ImageOptionalHeader(io::IO &io)
{
    magic                          = io.read_u16_le();
    major_linker_version           = io.read_u8();
    minor_linker_version           = io.read_u8();
    size_of_code                   = io.read_u32_le();
    size_of_initialized_data       = io.read_u32_le();
    size_of_uninitialized_data     = io.read_u32_le();
    address_of_entry_point         = io.read_u32_le();
    base_of_code                   = io.read_u32_le();
    base_of_data                   = io.read_u32_le();
    image_base                     = io.read_u32_le();
    section_alignment              = io.read_u32_le();
    file_alignment                 = io.read_u32_le();
    major_operating_system_version = io.read_u16_le();
    minor_operating_system_version = io.read_u16_le();
    major_image_version            = io.read_u16_le();
    minor_image_version            = io.read_u16_le();
    major_subsystem_version        = io.read_u16_le();
    minor_subsystem_version        = io.read_u16_le();
    win32_version_value            = io.read_u32_le();
    size_of_image                  = io.read_u32_le();
    size_of_headers                = io.read_u32_le();
    checksum                       = io.read_u32_le();
    subsystem                      = io.read_u16_le();
    dll_characteristics            = io.read_u16_le();
    bool pe64 = magic == 0x20B;
    if (pe64)
    {
        size_of_stack_reserve = io.read_u64_le();
        size_of_stack_commit  = io.read_u64_le();
        size_of_heap_reserve  = io.read_u64_le();
        size_of_heap_commit   = io.read_u64_le();
    }
    else
    {
        size_of_stack_reserve = io.read_u32_le();
        size_of_stack_commit  = io.read_u32_le();
        size_of_heap_reserve  = io.read_u32_le();
        size_of_heap_commit   = io.read_u32_le();
    }
    loader_flags = io.read_u32_le();
    number_of_rva_and_sizes = io.read_u32_le();
}

ImageFileHeader::ImageFileHeader(io::IO &io)
{
    machine = io.read_u16_le();
    number_of_sections = io.read_u16_le();
    timestamp = io.read_u32_le();
    pointer_to_symbol_table = io.read_u32_le();
    number_of_symbols = io.read_u32_le();
    size_of_optional_header = io.read_u16_le();
    characteristics = io.read_u16_le();
}

ImageNtHeader::ImageNtHeader(io::IO &io)
    : signature(io.read_u32_le()), file_header(io), optional_header(io)
{
}

ImageDataDir::ImageDataDir(io::IO &io)
{
    virtual_address = io.read_u32_le();
    size = io.read_u32_le();
}

ImageSectionHeader::ImageSectionHeader(io::IO &io)
{
    name                    = io.read(8).str();
    virtual_size            = io.read_u32_le();
    virtual_address         = io.read_u32_le();
    size_of_raw_data        = io.read_u32_le();
    pointer_to_raw_data     = io.read_u32_le();
    pointer_to_relocations  = io.read_u32_le();
    pointer_to_line_numbers = io.read_u32_le();
    number_of_relocations   = io.read_u16_le();
    number_of_line_numbers  = io.read_u16_le();
    characteristics         = io.read_u32_le();
}

ImageResourceDir::ImageResourceDir(io::IO &io)
{
    characteristics         = io.read_u32_le();
    timestamp               = io.read_u32_le();
    major_version           = io.read_u16_le();
    minor_version           = io.read_u16_le();
    number_of_named_entries = io.read_u16_le();
    number_of_id_entries    = io.read_u16_le();
}

ImageResourceDirEntry::ImageResourceDirEntry(io::IO &io)
{
    //i am ugliness
    name = io.read_u32_le();
    offset_to_data = io.read_u32_le();
    id = name;
    name_is_string = (name >> 31) > 0;
    name_offset = name & 0x7FFFFFFF;
    data_is_dir = offset_to_data >> 31;
    offset_to_data &= 0x7FFFFFFF;
}

ImageResourceDataEntry::ImageResourceDataEntry(io::IO &io)
{
    offset_to_data = io.read_u32_le();
    size = io.read_u32_le();
    code_page = io.read_u32_le();
    io.skip(4);
}

RvaHelper::RvaHelper(
    u32 file_alignment,
    u32 section_alignment,
    const std::vector<ImageSectionHeader> &sections)
: file_alignment(file_alignment),
    section_alignment(section_alignment),
    sections(sections)
{
}

u32 RvaHelper::rva_to_offset(u32 rva) const
{
    const ImageSectionHeader &section = section_for_rva(rva);
    return rva
        + adjust_file_alignment(section.pointer_to_raw_data)
        - adjust_section_alignment(section.virtual_address);
}

const ImageSectionHeader &RvaHelper::section_for_rva(u32 rva) const
{
    for (auto &section : sections)
    {
        if (rva >= section.virtual_address
        && rva < (section.virtual_address + section.virtual_size))
        {
            return section;
        }
    }
    throw err::CorruptDataError("Section not found");
}

u32 RvaHelper::adjust_file_alignment(u32 offset) const
{
    return file_alignment < 0x200 ? offset : (offset / 0x200) * 0x200;
}

u32 RvaHelper::adjust_section_alignment(u32 offset) const
{
    u32 fixed_alignment = section_alignment < 0x1000
        ? file_alignment
        : section_alignment;
    if (fixed_alignment && (offset % fixed_alignment))
        return fixed_alignment * (offset / fixed_alignment);
    return offset;
}

ResourceCrawlerArgs::ResourceCrawlerArgs(
    io::IO &io, FileSaver &saver, const RvaHelper &helper, size_t base_offset)
    : io(io), saver(saver), rva_helper(helper), base_offset(base_offset)
{
}

void ResourceCrawler::crawl(const ResourceCrawlerArgs &args)
{
    ResourceCrawler crawler(args);
    crawler.process_dir(0);
}

ResourceCrawler::ResourceCrawler(const ResourceCrawlerArgs &args) : args(args)
{
}

void ResourceCrawler::process_dir(size_t offset, const std::string path)
{
    args.io.seek(args.base_offset + offset);
    ImageResourceDir dir(args.io);
    size_t entry_count = dir.number_of_named_entries + dir.number_of_id_entries;
    for (auto i : util::range(entry_count))
    {
        ImageResourceDirEntry entry(args.io);

        try
        {
            args.io.peek(args.io.tell(), [&]()
            {
                std::string entry_path = read_entry_name(entry);
                if (path != "")
                    entry_path = path + path_sep + entry_path;

                if (entry.data_is_dir)
                    process_dir(entry.offset_to_data, entry_path);
                else
                    process_entry(entry.offset_to_data, entry_path);
            });
        }
        catch (std::exception &e)
        {
            Log.err(util::format(
                "Can't read resource entry located at 0x%08x (%s)",
                args.base_offset + entry.offset_to_data,
                e.what()));
        }
    }
}

void ResourceCrawler::process_entry(size_t offset, const std::string &path)
{
    args.io.seek(args.base_offset + offset);
    ImageResourceDataEntry entry(args.io);

    std::unique_ptr<File> file(new File);
    file->name = path;
    args.io.seek(args.rva_helper.rva_to_offset(entry.offset_to_data));
    file->io.write_from_io(args.io, entry.size);

    file->guess_extension();
    args.saver.save(std::move(file));
}

std::string ResourceCrawler::read_entry_name(const ImageResourceDirEntry &entry)
{
    if (entry.name_is_string)
    {
        args.io.seek(args.base_offset + entry.name_offset);
        size_t max_size = args.io.read_u16_le();
        bstr name_utf16 = args.io.read(max_size * 2);
        return util::convert_encoding(name_utf16, "utf-16le", "utf-8").str();
    }

    switch (entry.id)
    {
        case 1: return "CURSOR";
        case 2: return "BITMAP";
        case 3: return "ICON";
        case 4: return "MENU";
        case 5: return "DIALOG";
        case 6: return "STRING";
        case 7: return "FONT_DIRECTORY";
        case 8: return "FONT";
        case 9: return "ACCELERATOR";
        case 10: return "RC_DATA";
        case 11: return "MESSAGE_TABLE";
        case 16: return "VERSION";
        case 17: return "DLG_INCLUDE";
        case 19: return "PLUG_AND_PLAY";
        case 20: return "VXD";
        case 21: return "ANIMATED_CURSOR";
        case 22: return "ANIMATED_ICON";
        case 23: return "HTML";
        case 24: return "MANIFEST";
    }

    return util::format("%d", entry.id);
}

bool ExeArchive::is_recognized_internal(File &arc_file) const
{
    DosHeader dos_header(arc_file.io);
    return dos_header.magic == "MZ"_b;
}

void ExeArchive::unpack_internal(File &file, FileSaver &file_saver) const
{
    DosHeader dos_header(file.io);
    file.io.seek(dos_header.e_lfanew);
    ImageNtHeader nt_header(file.io);

    size_t data_dir_count = nt_header.optional_header.number_of_rva_and_sizes;
    std::vector<ImageDataDir> data_dirs;
    data_dirs.reserve(data_dir_count);
    for (auto i : util::range(data_dir_count))
        data_dirs.push_back(ImageDataDir(file.io));

    std::vector<ImageSectionHeader> sections;
    for (auto i : util::range(nt_header.file_header.number_of_sections))
        sections.push_back(ImageSectionHeader(file.io));

    RvaHelper rva_helper(
        nt_header.optional_header.file_alignment,
        nt_header.optional_header.section_alignment,
        sections);

    auto resource_dir = data_dirs[2];
    size_t base_offset = rva_helper.rva_to_offset(resource_dir.virtual_address);
    ResourceCrawler::crawl(
        ResourceCrawlerArgs(file.io, file_saver, rva_helper, base_offset));
}

static auto dummy = fmt::Registry::add<ExeArchive>("ms/exe");
