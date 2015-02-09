// EXE executable file
//
// Company:   Microsoft
// Engine:    Microsoft Windows
// Extension: .exe
//
// Known games:
// - Fortune Summoners: Secret Of The Elemental Stone

#include <cstdio>
#include "formats/arc/exe_archive.h"
#include "string_ex.h"

namespace
{
    const std::string path_sep = "／"; //keep flat hierarchy for unpacked files

    typedef struct DosHeader
    {
        std::string magic;
        uint16_t e_cblp;
        uint16_t e_cp;
        uint16_t e_crlc;
        uint16_t e_cparhdr;
        uint16_t e_minalloc;
        uint16_t e_maxalloc;
        uint16_t e_ss;
        uint16_t e_sp;
        uint16_t e_csum;
        uint16_t e_ip;
        uint16_t e_cs;
        uint16_t e_lfarlc;
        uint16_t e_ovno;
        uint16_t e_oemid;
        uint16_t e_oeminfo;
        uint32_t e_lfanew;

        DosHeader(IO &io)
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
    } DosHeader;

    typedef struct ImageOptionalHeader
    {
        uint16_t magic;
        uint8_t major_linker_version;
        uint8_t minor_linker_version;
        uint32_t size_of_code;
        uint32_t size_of_initialized_data;
        uint32_t size_of_uninitialized_data;
        uint32_t address_of_entry_point;
        uint32_t base_of_code;
        uint32_t base_of_data;
        uint32_t image_base;
        uint32_t section_alignment;
        uint32_t file_alignment;
        uint16_t major_operating_system_version;
        uint16_t minor_operating_system_version;
        uint16_t major_image_version;
        uint16_t minor_image_version;
        uint16_t major_subsystem_version;
        uint16_t minor_subsystem_version;
        uint32_t win32_version_value;
        uint32_t size_of_image;
        uint32_t size_of_headers;
        uint32_t checksum;
        uint16_t subsystem;
        uint16_t dll_characteristics;
        uint64_t size_of_stack_reserve;
        uint64_t size_of_stack_commit;
        uint64_t size_of_heap_reserve;
        uint64_t size_of_heap_commit;
        uint32_t loader_flags;
        uint32_t number_of_rva_and_sizes;

        ImageOptionalHeader(IO &io)
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
    } ImageOptionalHeader;

    typedef struct ImageFileHeader
    {
        uint16_t machine;
        uint16_t number_of_sections;
        uint32_t timestamp;
        uint32_t pointer_to_symbol_table;
        uint32_t number_of_symbols;
        uint16_t size_of_optional_header;
        uint16_t characteristics;

        ImageFileHeader(IO &io)
        {
            machine = io.read_u16_le();
            number_of_sections = io.read_u16_le();
            timestamp = io.read_u32_le();
            pointer_to_symbol_table = io.read_u32_le();
            number_of_symbols = io.read_u32_le();
            size_of_optional_header = io.read_u16_le();
            characteristics = io.read_u16_le();
        }
    } ImageFileHeader;

    typedef struct ImageNtHeader
    {
        uint32_t signature;
        ImageFileHeader file_header;
        ImageOptionalHeader optional_header;

        ImageNtHeader(IO &io)
        : signature(io.read_u32_le()),
            file_header(io),
            optional_header(io)
        {
        }
    } ImageNtHeader;

    typedef struct ImageDataDirectory
    {
        uint32_t virtual_address;
        uint32_t size;

        ImageDataDirectory(IO &io)
        {
            virtual_address = io.read_u32_le();
            size = io.read_u32_le();
        }
    } ImageDataDriectory;

    typedef struct ImageSectionHeader
    {
        std::string name;
        uint32_t virtual_size;
        uint32_t physical_address;
        uint32_t virtual_address;
        uint32_t size_of_raw_data;
        uint32_t pointer_to_raw_data;
        uint32_t pointer_to_relocations;
        uint32_t pointer_to_line_numbers;
        uint16_t number_of_relocations;
        uint16_t number_of_line_numbers;
        uint32_t characteristics;

        ImageSectionHeader(IO &io)
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
    } ImageSectionHeader;

    typedef struct ImageResourceDirectory
    {
        uint32_t characteristics;
        uint32_t timestamp;
        uint16_t major_version;
        uint16_t minor_version;
        uint16_t number_of_named_entries;
        uint16_t number_of_id_entries;

        ImageResourceDirectory(IO &io)
        {
            characteristics         = io.read_u32_le();
            timestamp               = io.read_u32_le();
            major_version           = io.read_u16_le();
            minor_version           = io.read_u16_le();
            number_of_named_entries = io.read_u16_le();
            number_of_id_entries    = io.read_u16_le();
        }
    } ImageResourceDirectory;

    typedef struct ImageResourceDirectoryEntry
    {
        uint32_t offset_to_data;
        bool name_is_string;
        uint32_t name_offset;
        uint32_t name;
        uint32_t id;
        uint32_t data_is_directory;

        ImageResourceDirectoryEntry(IO &io)
        {
            //i am ugliness
            // possibly replace with unions?
            name = io.read_u32_le();
            offset_to_data = io.read_u32_le();
            id = name;
            name_is_string = name >> 31;
            name_offset = name & 0x7fffffff;
            data_is_directory = offset_to_data >> 31;
            offset_to_data &= 0x7fffffff;
        }
    } ImageResourceDirectoryEntry;

    typedef struct ImageResourceDataEntry
    {
        uint32_t offset_to_data;
        uint32_t size;
        uint32_t code_page;

        ImageResourceDataEntry(IO &io)
        {
            offset_to_data = io.read_u32_le();
            size = io.read_u32_le();
            code_page = io.read_u32_le();
            io.skip(4);
        }
    } ImageResourceDataEntry;

    class RvaHelper
    {
    private:
        uint32_t file_alignment;
        uint32_t section_alignment;

        const ImageSectionHeader &section_for_rva(
            const std::vector<ImageSectionHeader> &sections,
            uint32_t rva)
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

        uint32_t adjust_file_alignment(uint32_t offset)
        {
            return file_alignment < 0x200 ? offset : (offset / 0x200) * 0x200;
        }

        uint32_t adjust_section_alignment(uint32_t offset)
        {
            uint32_t fixed_alignment
                = section_alignment < 0x1000
                    ? file_alignment
                    : section_alignment;
            if (fixed_alignment && (offset % fixed_alignment))
                return fixed_alignment * (offset / fixed_alignment);
            return offset;
        }

    public:
        RvaHelper(uint32_t file_alignment, uint32_t section_alignment)
        : file_alignment(file_alignment), section_alignment(section_alignment)
        {
        }

        uint32_t rva_to_offset(
            const std::vector<ImageSectionHeader> &sections,
            const uint32_t rva)
        {
            const ImageSectionHeader &section = section_for_rva(sections, rva);
            return rva
                + adjust_file_alignment(section.pointer_to_raw_data)
                - adjust_section_alignment(section.virtual_address);
        }
    };

    std::string read_entry_name(
        IO &io,
        size_t base_offset,
        const ImageResourceDirectoryEntry &entry)
    {
        if (entry.name_is_string)
        {
            io.seek(base_offset + entry.name_offset);
            size_t max_length = io.read_u16_le();
            std::string utf16le = io.read(max_length * 2);
            return convert_encoding(utf16le, "utf-16le", "utf-8");
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

        //no std::stoi on cygwin and mingw
        char x[10];
        sprintf(x, "%d", entry.id);
        return std::string(x);
    }

    void process_image_resource_data_entry(
        IO &io,
        size_t base_offset,
        size_t offset,
        std::vector<ImageSectionHeader> &sections,
        RvaHelper &rva_helper,
        OutputFiles &output_files,
        std::string path)
    {
        io.seek(base_offset + offset);
        ImageResourceDataEntry entry(io);
        output_files.save([&]() -> std::unique_ptr<VirtualFile>
        {
            std::unique_ptr<VirtualFile> file(new VirtualFile);
            file->name = path;
            io.seek(rva_helper.rva_to_offset(sections, entry.offset_to_data));
            file->io.write_from_io(io, entry.size);
            return file;
        });
    }

    void process_image_resource_directory(
        IO &io,
        size_t base_offset,
        size_t offset,
        std::vector<ImageSectionHeader> &sections,
        RvaHelper &rva_helper,
        OutputFiles &output_files,
        const std::string path = "")
    {
        io.seek(base_offset + offset);
        ImageResourceDirectory image_resource_directory(io);
        size_t entry_count =
            image_resource_directory.number_of_named_entries +
            image_resource_directory.number_of_id_entries;
        size_t directory_entry_offset = io.tell() - base_offset;
        for (size_t i = 0; i < entry_count; i ++)
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
                    output_files,
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
                    output_files,
                    entry_path);
            }
            directory_entry_offset += 8;
        }
    }
}

void ExeArchive::unpack_internal(IO &arc_io, OutputFiles &output_files) const
{
    DosHeader dos_header(arc_io);
    if (dos_header.magic != "MZ")
        throw  std::runtime_error("Not an EXE executable");

    arc_io.seek(dos_header.e_lfanew);
    ImageNtHeader nt_header(arc_io);

    RvaHelper rva_helper(
        nt_header.optional_header.file_alignment,
        nt_header.optional_header.section_alignment);

    size_t image_data_directory_count
        = nt_header.optional_header.number_of_rva_and_sizes;
    std::vector<ImageDataDirectory> image_data_directories;
    image_data_directories.reserve(image_data_directory_count);
    for (size_t i = 0; i < image_data_directory_count; i ++)
        image_data_directories.push_back(ImageDataDirectory(arc_io));

    std::vector<ImageSectionHeader> sections;
    for (size_t i = 0; i < nt_header.file_header.number_of_sections; i ++)
        sections.push_back(ImageSectionHeader(arc_io));

    auto resource_directory = image_data_directories[2];
    size_t offset_to_image_resources = rva_helper.rva_to_offset(
        sections, resource_directory.virtual_address);

    process_image_resource_directory(
        arc_io,
        offset_to_image_resources,
        0,
        sections,
        rva_helper,
        output_files);
}
