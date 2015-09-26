#pragma once

#include "abstract_decoder.h"

namespace au {
namespace fmt {

    class ArchiveDecoder : public AbstractDecoder
    {
    public:
        virtual ~ArchiveDecoder();
        virtual void register_cli_options(ArgParser &) const override;
        virtual void parse_cli_options(const ArgParser &) override;
        virtual FileNamingStrategy get_file_naming_strategy() const override;
        virtual void unpack(File &, FileSaver &, bool) const override;

    protected:
        virtual void unpack_internal(File &, FileSaver &) const = 0;
        void add_decoder(AbstractDecoder *decoder);

    private:
        std::vector<AbstractDecoder*> decoders;
    };

} }
