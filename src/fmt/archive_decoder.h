#pragma once

#include "idecoder.h"

namespace au {
namespace fmt {

    struct ArchiveEntry
    {
        virtual ~ArchiveEntry() { }
        std::string name;
    };

    struct ArchiveMeta
    {
        virtual ~ArchiveMeta() { }
        std::vector<std::unique_ptr<ArchiveEntry>> entries;
    };

    class ArchiveDecoder : public IDecoder
    {
    public:
        ArchiveDecoder();
        virtual ~ArchiveDecoder();

        virtual void register_cli_options(ArgParser &) const override;
        virtual void parse_cli_options(const ArgParser &) override;
        bool is_recognized(File &) const override;
        void unpack(File &, const FileSaver &) const override;
        virtual std::unique_ptr<INamingStrategy> naming_strategy()
            const override;

        void disable_preprocessing();
        virtual std::vector<std::string> get_linked_formats() const;

        std::unique_ptr<ArchiveMeta> read_meta(File &) const;
        std::unique_ptr<File> read_file(
            File &, const ArchiveMeta &, const ArchiveEntry &) const;

    protected:
        virtual bool is_recognized_impl(File &) const = 0;
        virtual std::unique_ptr<ArchiveMeta> read_meta_impl(File &) const = 0;
        virtual std::unique_ptr<File> read_file_impl(
            File &, const ArchiveMeta &, const ArchiveEntry &) const = 0;
        virtual void preprocess(File &, ArchiveMeta &, const FileSaver &) const;

    private:
        bool preprocessing_disabled;
    };

} }
