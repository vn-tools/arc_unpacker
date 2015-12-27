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
        virtual ~ArchiveDecoder() { }

        virtual NamingStrategy naming_strategy() const override;

        void accept(IDecoderVisitor &visitor) const override;

        std::unique_ptr<ArchiveMeta> read_meta(
            const Logger &logger, io::File &input_file) const;

        std::unique_ptr<io::File> read_file(
            const Logger &logger,
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const;

    protected:
        virtual std::unique_ptr<ArchiveMeta> read_meta_impl(
            const Logger &logger,
            io::File &input_file) const = 0;

        virtual std::unique_ptr<io::File> read_file_impl(
            const Logger &logger,
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const = 0;
    };

} }
