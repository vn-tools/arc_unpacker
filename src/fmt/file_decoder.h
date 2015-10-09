#pragma once

#include "idecoder.h"

namespace au {
namespace fmt {

    class FileDecoder : public IDecoder
    {
    public:
        virtual ~FileDecoder();
        virtual void register_cli_options(ArgParser &) const override;
        virtual void parse_cli_options(const ArgParser &) override;
        virtual bool is_recognized(File &) const override;
        virtual void unpack(File &, const FileSaver &) const override;
        virtual std::unique_ptr<INamingStrategy> naming_strategy()
            const override;

        std::unique_ptr<File> decode(File &) const;

    protected:
        virtual bool is_recognized_impl(File &) const = 0;
        virtual std::unique_ptr<File> decode_impl(File &) const = 0;
    };

} }
