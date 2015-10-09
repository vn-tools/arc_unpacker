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
        virtual bool is_recognized(File &) const override;
        virtual void unpack(File &, const FileSaver &) const override;
        virtual std::unique_ptr<INamingStrategy> naming_strategy()
            const override;

        void disable_nested_decoding();
        std::vector<std::shared_ptr<File>> unpack(File &) const;
        std::unique_ptr<ArchiveMeta> read_meta(File &) const;
        std::unique_ptr<File> read_file(
            File &, const ArchiveMeta &, const ArchiveEntry &) const;

    protected:
        virtual std::unique_ptr<ArchiveMeta> read_meta_impl(File &) const = 0;
        virtual std::unique_ptr<File> read_file_impl(
            File &, const ArchiveMeta &, const ArchiveEntry &) const = 0;
        virtual void preprocess(File &, ArchiveMeta &, const FileSaver &) const;
        virtual bool is_recognized_impl(File &) const = 0;
        void add_decoder(IDecoder *decoder);

        bool nested_decoding_enabled;

    private:
        std::vector<IDecoder*> decoders;
    };

} }
