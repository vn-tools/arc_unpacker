#pragma once

#include "idecoder.h"

namespace au {
namespace fmt {

    struct ArchiveEntry
    {
        virtual ~ArchiveEntry() { }
        io::path path;
    };

    struct ArchiveMeta
    {
        virtual ~ArchiveMeta() { }
        std::vector<std::unique_ptr<ArchiveEntry>> entries;
    };

    class ArchiveDecoder : public BaseDecoder
    {
    public:
        ArchiveDecoder();
        virtual ~ArchiveDecoder();

        void unpack(
            io::File &input_file,
            const FileSaver &file_saver) const override;

        virtual NamingStrategy naming_strategy() const override;

        void disable_preprocessing();

        virtual std::vector<std::string> get_linked_formats() const;

        std::unique_ptr<ArchiveMeta> read_meta(io::File &input_file) const;

        std::unique_ptr<io::File> read_file(
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const;

    protected:
        virtual std::unique_ptr<ArchiveMeta> read_meta_impl(
            io::File &input_file) const = 0;

        virtual std::unique_ptr<io::File> read_file_impl(
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const = 0;

        virtual void preprocess(
            io::File &input_file,
            ArchiveMeta &m,
            const FileSaver &file_saver) const;

    private:
        bool preprocessing_disabled;
    };

} }
