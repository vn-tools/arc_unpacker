#pragma once

#include "idecoder.h"

namespace au {
namespace fmt {

    class ArchiveDecoder : public IDecoder
    {
    public:
        ArchiveDecoder();
        virtual ~ArchiveDecoder();
        virtual void register_cli_options(ArgParser &) const override;
        virtual void parse_cli_options(const ArgParser &) override;
        virtual bool is_recognized(File &) const override;
        virtual void unpack(File &, FileSaver &) const override;
        virtual std::unique_ptr<INamingStrategy> naming_strategy()
            const override;

        void disable_nested_decoding();
        std::vector<std::shared_ptr<File>> unpack(File &) const;

    protected:
        virtual bool is_recognized_internal(File &) const = 0;
        virtual void unpack_internal(File &, FileSaver &) const = 0;
        void add_decoder(IDecoder *decoder);

        bool nested_decoding_enabled;

    private:
        std::vector<IDecoder*> decoders;
    };

} }
