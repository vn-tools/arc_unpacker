#include "fmt/microsoft/exe_archive_decoder.h"
#include "algo/format.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "err.h"
#include "log.h"

using namespace au;
using namespace au::fmt::microsoft;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };

    struct DosHeader final
    {
        DosHeader(io::Stream &stream);

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
    };

    struct ImageOptionalHeader final
    {
        ImageOptionalHeader(io::Stream &stream);

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
    };

    struct ImageFileHeader final
    {
        ImageFileHeader(io::Stream &stream);

        u16 machine;
        u16 number_of_sections;
        u32 timestamp;
        u32 pointer_to_symbol_table;
        u32 number_of_symbols;
        u16 size_of_optional_header;
        u16 characteristics;
    };

    struct ImageNtHeader final
    {
        ImageNtHeader(io::Stream &stream);

        u32 signature;
        ImageFileHeader file_header;
        ImageOptionalHeader optional_header;
    };

    struct ImageDataDir final
    {
        ImageDataDir(io::Stream &stream);

        u32 virtual_address;
        u32 size;
    };

    struct ImageSectionHeader final
    {
        ImageSectionHeader(io::Stream &stream);

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
    };

    struct ImageResourceDir final
    {
        ImageResourceDir(io::Stream &stream);

        u32 characteristics;
        u32 timestamp;
        u16 major_version;
        u16 minor_version;
        u16 number_of_named_entries;
        u16 number_of_id_entries;
    };

    struct ImageResourceDirEntry final
    {
        ImageResourceDirEntry(io::Stream &stream);

        u32 offset_to_data;
        bool name_is_string;
        u32 name_offset;
        u32 name;
        u32 id;
        u32 data_is_dir;
    };

    struct ImageResourceDataEntry final
    {
        ImageResourceDataEntry(io::Stream &stream);

        u32 offset_to_data;
        u32 size;
        u32 code_page;
    };

    class RvaHelper final
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

    struct ResourceCrawlerArgs final
    {
        ResourceCrawlerArgs(
            io::Stream &, fmt::ArchiveMeta &, const RvaHelper &, size_t);

        io::Stream &stream;
        fmt::ArchiveMeta &meta;
        const RvaHelper &rva_helper;
        size_t base_offset;
    };

    class ResourceCrawler final
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

// keep flat hierarchy for unpacked files
static const std::string path_sep = "／";

DosHeader::DosHeader(io::Stream &stream)
{
    magic      = stream.read(2);
    e_cblp     = stream.read_u16_le();
    e_cp       = stream.read_u16_le();
    e_crlc     = stream.read_u16_le();
    e_cparhdr  = stream.read_u16_le();
    e_minalloc = stream.read_u16_le();
    e_maxalloc = stream.read_u16_le();
    e_ss       = stream.read_u16_le();
    e_sp       = stream.read_u16_le();
    e_csum     = stream.read_u16_le();
    e_ip       = stream.read_u16_le();
    e_cs       = stream.read_u16_le();
    e_lfarlc   = stream.read_u16_le();
    e_ovno     = stream.read_u16_le();
    stream.skip(2 * 4);
    e_oemid    = stream.read_u16_le();
    e_oeminfo  = stream.read_u16_le();
    stream.skip(2 * 10);
    e_lfanew   = stream.read_u32_le();
}

ImageOptionalHeader::ImageOptionalHeader(io::Stream &stream)
{
    magic                          = stream.read_u16_le();
    major_linker_version           = stream.read_u8();
    minor_linker_version           = stream.read_u8();
    size_of_code                   = stream.read_u32_le();
    size_of_initialized_data       = stream.read_u32_le();
    size_of_uninitialized_data     = stream.read_u32_le();
    address_of_entry_point         = stream.read_u32_le();
    base_of_code                   = stream.read_u32_le();
    base_of_data                   = stream.read_u32_le();
    image_base                     = stream.read_u32_le();
    section_alignment              = stream.read_u32_le();
    file_alignment                 = stream.read_u32_le();
    major_operating_system_version = stream.read_u16_le();
    minor_operating_system_version = stream.read_u16_le();
    major_image_version            = stream.read_u16_le();
    minor_image_version            = stream.read_u16_le();
    major_subsystem_version        = stream.read_u16_le();
    minor_subsystem_version        = stream.read_u16_le();
    win32_version_value            = stream.read_u32_le();
    size_of_image                  = stream.read_u32_le();
    size_of_headers                = stream.read_u32_le();
    checksum                       = stream.read_u32_le();
    subsystem                      = stream.read_u16_le();
    dll_characteristics            = stream.read_u16_le();
    bool pe64 = magic == 0x20B;
    if (pe64)
    {
        size_of_stack_reserve = stream.read_u64_le();
        size_of_stack_commit  = stream.read_u64_le();
        size_of_heap_reserve  = stream.read_u64_le();
        size_of_heap_commit   = stream.read_u64_le();
    }
    else
    {
        size_of_stack_reserve = stream.read_u32_le();
        size_of_stack_commit  = stream.read_u32_le();
        size_of_heap_reserve  = stream.read_u32_le();
        size_of_heap_commit   = stream.read_u32_le();
    }
    loader_flags = stream.read_u32_le();
    number_of_rva_and_sizes = stream.read_u32_le();
}

ImageFileHeader::ImageFileHeader(io::Stream &stream)
{
    machine = stream.read_u16_le();
    number_of_sections = stream.read_u16_le();
    timestamp = stream.read_u32_le();
    pointer_to_symbol_table = stream.read_u32_le();
    number_of_symbols = stream.read_u32_le();
    size_of_optional_header = stream.read_u16_le();
    characteristics = stream.read_u16_le();
}

ImageNtHeader::ImageNtHeader(io::Stream &stream) :
    signature(stream.read_u32_le()),
    file_header(stream),
    optional_header(stream)
{
}

ImageDataDir::ImageDataDir(io::Stream &stream)
{
    virtual_address = stream.read_u32_le();
    size = stream.read_u32_le();
}

ImageSectionHeader::ImageSectionHeader(io::Stream &stream)
{
    name                    = stream.read(8).str();
    virtual_size            = stream.read_u32_le();
    virtual_address         = stream.read_u32_le();
    size_of_raw_data        = stream.read_u32_le();
    pointer_to_raw_data     = stream.read_u32_le();
    pointer_to_relocations  = stream.read_u32_le();
    pointer_to_line_numbers = stream.read_u32_le();
    number_of_relocations   = stream.read_u16_le();
    number_of_line_numbers  = stream.read_u16_le();
    characteristics         = stream.read_u32_le();
}

ImageResourceDir::ImageResourceDir(io::Stream &stream)
{
    characteristics         = stream.read_u32_le();
    timestamp               = stream.read_u32_le();
    major_version           = stream.read_u16_le();
    minor_version           = stream.read_u16_le();
    number_of_named_entries = stream.read_u16_le();
    number_of_id_entries    = stream.read_u16_le();
}

ImageResourceDirEntry::ImageResourceDirEntry(io::Stream &stream)
{
    // I am ugliness
    name = stream.read_u32_le();
    offset_to_data = stream.read_u32_le();
    id = name;
    name_is_string = (name >> 31) > 0;
    name_offset = name & 0x7FFFFFFF;
    data_is_dir = offset_to_data >> 31;
    offset_to_data &= 0x7FFFFFFF;
}

ImageResourceDataEntry::ImageResourceDataEntry(io::Stream &stream)
{
    offset_to_data = stream.read_u32_le();
    size = stream.read_u32_le();
    code_page = stream.read_u32_le();
    stream.skip(4);
}

RvaHelper::RvaHelper(
    u32 file_alignment,
    u32 section_alignment,
    const std::vector<ImageSectionHeader> &sections) :
        file_alignment(file_alignment),
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
    io::Stream &stream,
    fmt::ArchiveMeta &meta,
    const RvaHelper &helper,
    size_t base_offset)
    : stream(stream), meta(meta), rva_helper(helper), base_offset(base_offset)
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
    args.stream.seek(args.base_offset + offset);
    ImageResourceDir dir(args.stream);
    size_t entry_count = dir.number_of_named_entries + dir.number_of_id_entries;
    for (auto i : algo::range(entry_count))
    {
        ImageResourceDirEntry entry(args.stream);

        try
        {
            args.stream.peek(args.stream.tell(), [&]()
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
            Log.err(algo::format(
                "Can't read resource entry located at 0x%08x (%s)\n",
                args.base_offset + entry.offset_to_data,
                e.what()));
        }
    }
}

void ResourceCrawler::process_entry(size_t offset, const std::string &path)
{
    args.stream.seek(args.base_offset + offset);
    ImageResourceDataEntry resource_entry(args.stream);

    auto entry = std::make_unique<ArchiveEntryImpl>();
    entry->path = path;
    entry->offset = args.rva_helper.rva_to_offset(
        resource_entry.offset_to_data);
    entry->size = resource_entry.size;
    args.meta.entries.push_back(std::move(entry));
}

std::string ResourceCrawler::read_entry_name(const ImageResourceDirEntry &entry)
{
    if (entry.name_is_string)
    {
        args.stream.seek(args.base_offset + entry.name_offset);
        size_t max_size = args.stream.read_u16_le();
        bstr name_utf16 = args.stream.read(max_size * 2);
        return algo::utf16_to_utf8(name_utf16).str();
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

    return algo::format("%d", entry.id);
}

bool ExeArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    DosHeader dos_header(input_file.stream);
    return dos_header.magic == "MZ"_b;
}

std::unique_ptr<fmt::ArchiveMeta>
    ExeArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    DosHeader dos_header(input_file.stream);
    input_file.stream.seek(dos_header.e_lfanew);
    ImageNtHeader nt_header(input_file.stream);

    size_t data_dir_count = nt_header.optional_header.number_of_rva_and_sizes;
    std::vector<ImageDataDir> data_dirs;
    data_dirs.reserve(data_dir_count);
    for (auto i : algo::range(data_dir_count))
        data_dirs.push_back(ImageDataDir(input_file.stream));

    std::vector<ImageSectionHeader> sections;
    for (auto i : algo::range(nt_header.file_header.number_of_sections))
        sections.push_back(ImageSectionHeader(input_file.stream));

    RvaHelper rva_helper(
        nt_header.optional_header.file_alignment,
        nt_header.optional_header.section_alignment,
        sections);

    auto resource_dir = data_dirs[2];
    size_t base_offset = rva_helper.rva_to_offset(resource_dir.virtual_address);
    auto meta = std::make_unique<ArchiveMeta>();
    ResourceCrawler::crawl(
        ResourceCrawlerArgs(input_file.stream, *meta, rva_helper, base_offset));
    return meta;
}

std::unique_ptr<io::File> ExeArchiveDecoder::read_file_impl(
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<ExeArchiveDecoder>("microsoft/exe");
