#pragma once

#include "idecoder.h"

namespace au {
namespace fmt {

    class FileDecoder : public IDecoder
    {
    public:
        virtual ~FileDecoder();
        void register_cli_options(ArgParser &) const override;
        void parse_cli_options(const ArgParser &) override;
        bool is_recognized(File &) const override;
        void unpack(File &, const FileSaver &) const override;
        std::unique_ptr<INamingStrategy> naming_strategy() const override;

        std::unique_ptr<File> decode(File &) const;

    protected:
        virtual bool is_recognized_impl(File &) const = 0;
        virtual std::unique_ptr<File> decode_impl(File &) const = 0;
    };

} }
