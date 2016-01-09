#pragma once

#include <map>
#include "dec/base_archive_decoder.h"

namespace au {
namespace dec {
namespace whale {

    class DatArchiveDecoder final : public BaseArchiveDecoder
    {
    public:
        DatArchiveDecoder();
        void set_game_title(const std::string &game_title);
        void add_file_name(const std::string &file_name);
        std::vector<std::string> get_linked_formats() const override;

    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<ArchiveMeta> read_meta_impl(
            const Logger &logger,
            io::File &input_file) const override;

        std::unique_ptr<io::File> read_file_impl(
            const Logger &logger,
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const override;

    private:
        bstr game_title;
        std::map<u64, bstr> file_names_map;
        std::string dump_path;
    };

} } }
