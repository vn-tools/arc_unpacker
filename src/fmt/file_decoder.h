#pragma once

#include "idecoder.h"

namespace au {
namespace fmt {

    class FileDecoder : public IDecoder
    {
    public:
        virtual ~FileDecoder();
        virtual FileNamingStrategy get_file_naming_strategy() const override;

        virtual void register_cli_options(ArgParser &) const override;
        virtual void parse_cli_options(const ArgParser &) override;

        virtual bool is_recognized(File &) const override;
        virtual void unpack(File &, FileSaver &, bool) const override;

        std::unique_ptr<File> decode(File &) const;

    protected:
        virtual bool is_recognized_internal(File &) const = 0;
        virtual std::unique_ptr<File> decode_internal(File &) const = 0;
    };

} }
