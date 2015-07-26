// EXE executable file
//
// Company:   Microsoft
// Engine:    Microsoft Windows
// Extension: .exe
//
// Known games:
// - Fortune Summoners: Secret Of The Elemental Stone

#include <cstdio>
#include "formats/microsoft/exe_archive.h"
#include "util/encoding.h"
#include "util/itos.h"

using namespace au;
using namespace au::fmt::microsoft;

namespace
{
    typedef struct DosHeader
    {
        std::string magic;
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
    } DosHeader;

    typedef struct ImageOptionalHeader
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
    } ImageOptionalHeader;

    typedef struct ImageFileHeader
    {
        u16 machine;
        u16 number_of_sections;
        u32 timestamp;
        u32 pointer_to_symbol_table;
        u32 number_of_symbols;
        u16 size_of_optional_header;
        u16 characteristics;
        ImageFileHeader(io::IO &io);
    } ImageFileHeader;

    typedef struct ImageNtHeader
    {
        u32 signature;
        ImageFileHeader file_header;
        ImageOptionalHeader optional_header;
        ImageNtHeader(io::IO &io);
    } ImageNtHeader;

    typedef struct ImageDataDirectory
    {
        u32 virtual_address;
        u32 size;
        ImageDataDirectory(io::IO &io);
    } ImageDataDriectory;

    typedef struct ImageSectionHeader
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
    } ImageSectionHeader;

    typedef struct ImageResourceDirectory
    {
        u32 characteristics;
        u32 timestamp;
        u16 major_version;
        u16 minor_version;
        u16 number_of_named_entries;
        u16 number_of_id_entries;
        ImageResourceDirectory(io::IO &io);
    } ImageResourceDirectory;

    typedef struct ImageResourceDirectoryEntry
    {
        u32 offset_to_data;
        bool name_is_string;
        u32 name_offset;
        u32 name;
        u32 id;
        u32 data_is_directory;
        ImageResourceDirectoryEntry(io::IO &io);
    } ImageResourceDirectoryEntry;

    typedef struct ImageResourceDataEntry
    {
        u32 offset_to_data;
        u32 size;
        u32 code_page;
        ImageResourceDataEntry(io::IO &io);
    } ImageResourceDataEntry;

    class RvaHelper
    {
    public:
        RvaHelper(u32 file_alignment, u32 section_alignment);
        u32 rva_to_offset(
            const std::vector<ImageSectionHeader> &sections, u32 rva);
    private:
        u32 file_alignment;
        u32 section_alignment;

        const ImageSectionHeader &section_for_rva(
            const std::vector<ImageSectionHeader> &sections, u32 rva);

        u32 adjust_file_alignment(u32 offset);
        u32 adjust_section_alignment(u32 offset);
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
    bool pe64 = magic == 0x20b;
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

ImageDataDirectory::ImageDataDirectory(io::IO &io)
{
    virtual_address = io.read_u32_le();
    size = io.read_u32_le();
}

ImageSectionHeader::ImageSectionHeader(io::IO &io)
{
    name                    = io.read(8);
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

ImageResourceDirectory::ImageResourceDirectory(io::IO &io)
{
    characteristics         = io.read_u32_le();
    timestamp               = io.read_u32_le();
    major_version           = io.read_u16_le();
    minor_version           = io.read_u16_le();
    number_of_named_entries = io.read_u16_le();
    number_of_id_entries    = io.read_u16_le();
}

ImageResourceDirectoryEntry::ImageResourceDirectoryEntry(io::IO &io)
{
    //i am ugliness
    name = io.read_u32_le();
    offset_to_data = io.read_u32_le();
    id = name;
    name_is_string = (name >> 31) > 0;
    name_offset = name & 0x7fffffff;
    data_is_directory = offset_to_data >> 31;
    offset_to_data &= 0x7fffffff;
}

ImageResourceDataEntry::ImageResourceDataEntry(io::IO &io)
{
    offset_to_data = io.read_u32_le();
    size = io.read_u32_le();
    code_page = io.read_u32_le();
    io.skip(4);
}

const ImageSectionHeader &RvaHelper::section_for_rva(
    const std::vector<ImageSectionHeader> &sections, u32 rva)
{
    for (auto &section : sections)
    {
        if (rva >= section.virtual_address
        && rva <= (section.virtual_address + section.virtual_size))
        {
            return section;
        }
    }
    throw std::runtime_error("Section not found");
}

u32 RvaHelper::adjust_file_alignment(u32 offset)
{
    return file_alignment < 0x200 ? offset : (offset / 0x200) * 0x200;
}

u32 RvaHelper::adjust_section_alignment(u32 offset)
{
    u32 fixed_alignment
        = section_alignment < 0x1000
            ? file_alignment
            : section_alignment;
    if (fixed_alignment && (offset % fixed_alignment))
        return fixed_alignment * (offset / fixed_alignment);
    return offset;
}

RvaHelper::RvaHelper(u32 file_alignment, u32 section_alignment)
    : file_alignment(file_alignment), section_alignment(section_alignment)
{
}

u32 RvaHelper::rva_to_offset(
    const std::vector<ImageSectionHeader> &sections, u32 rva)
{
    const ImageSectionHeader &section = section_for_rva(sections, rva);
    return rva
        + adjust_file_alignment(section.pointer_to_raw_data)
        - adjust_section_alignment(section.virtual_address);
}

static std::string read_entry_name(
    io::IO &io, size_t base_offset, const ImageResourceDirectoryEntry &entry)
{
    if (entry.name_is_string)
    {
        io.seek(base_offset + entry.name_offset);
        size_t max_length = io.read_u16_le();
        std::string utf16le = io.read(max_length * 2);
        return util::convert_encoding(utf16le, "utf-16le", "utf-8");
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

    return util::itos(entry.id);
}

static void process_image_resource_data_entry(
    io::IO &io,
    size_t base_offset,
    size_t offset,
    std::vector<ImageSectionHeader> &sections,
    RvaHelper &rva_helper,
    FileSaver &file_saver,
    std::string path)
{
    io.seek(base_offset + offset);
    ImageResourceDataEntry entry(io);

    std::unique_ptr<File> file(new File);
    file->name = path;
    io.seek(rva_helper.rva_to_offset(sections, entry.offset_to_data));
    file->io.write_from_io(io, entry.size);

    file->guess_extension();
    file_saver.save(std::move(file));
}

static void process_image_resource_directory(
    io::IO &io,
    size_t base_offset,
    size_t offset,
    std::vector<ImageSectionHeader> &sections,
    RvaHelper &rva_helper,
    FileSaver &file_saver,
    const std::string path = "")
{
    io.seek(base_offset + offset);
    ImageResourceDirectory image_resource_directory(io);
    size_t entry_count =
        image_resource_directory.number_of_named_entries +
        image_resource_directory.number_of_id_entries;
    size_t directory_entry_offset = io.tell() - base_offset;
    for (size_t i = 0; i < entry_count; i++)
    {
        io.seek(base_offset + directory_entry_offset);
        ImageResourceDirectoryEntry entry(io);
        std::string entry_path = read_entry_name(io, base_offset, entry);
        if (path != "")
        {
            entry_path = path + path_sep + entry_path;
        }
        if (entry.data_is_directory)
        {
            //another directory
            process_image_resource_directory(
                io,
                base_offset,
                entry.offset_to_data,
                sections,
                rva_helper,
                file_saver,
                entry_path);
        }
        else
        {
            //file
            process_image_resource_data_entry(
                io,
                base_offset,
                entry.offset_to_data,
                sections,
                rva_helper,
                file_saver,
                entry_path);
        }
        directory_entry_offset += 8;
    }
}

bool ExeArchive::is_recognized_internal(File &arc_file) const
{
    DosHeader dos_header(arc_file.io);
    return dos_header.magic == "MZ";
}

void ExeArchive::unpack_internal(File &file, FileSaver &file_saver) const
{
    DosHeader dos_header(file.io);
    file.io.seek(dos_header.e_lfanew);
    ImageNtHeader nt_header(file.io);

    RvaHelper rva_helper(
        nt_header.optional_header.file_alignment,
        nt_header.optional_header.section_alignment);

    size_t image_data_directory_count
        = nt_header.optional_header.number_of_rva_and_sizes;
    std::vector<ImageDataDirectory> image_data_directories;
    image_data_directories.reserve(image_data_directory_count);
    for (size_t i = 0; i < image_data_directory_count; i++)
        image_data_directories.push_back(ImageDataDirectory(file.io));

    std::vector<ImageSectionHeader> sections;
    for (size_t i = 0; i < nt_header.file_header.number_of_sections; i++)
        sections.push_back(ImageSectionHeader(file.io));

    auto resource_directory = image_data_directories[2];
    size_t offset_to_image_resources = rva_helper.rva_to_offset(
        sections, resource_directory.virtual_address);

    process_image_resource_directory(
        file.io,
        offset_to_image_resources,
        0,
        sections,
        rva_helper,
        file_saver);
}
