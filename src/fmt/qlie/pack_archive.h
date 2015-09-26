#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace qlie {

    class PackArchive final : public ArchiveDecoder
    {
    public:
        PackArchive();
        ~PackArchive();
        void register_cli_options(ArgParser &) const override;
        void parse_cli_options(const ArgParser &) override;
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
