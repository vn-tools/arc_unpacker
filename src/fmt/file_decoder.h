#pragma once

#include "idecoder.h"

namespace au {
namespace fmt {

    class FileDecoder : public IDecoder
    {
    public:
        virtual ~FileDecoder();

        void register_cli_options(ArgParser &arg_parser) const override;
        void parse_cli_options(const ArgParser &arg_parser) override;

        bool is_recognized(io::File &input_file) const override;

        void unpack(
            io::File &input_file,
            const FileSaver &file_saver) const override;

        std::unique_ptr<INamingStrategy> naming_strategy() const override;

        std::unique_ptr<io::File> decode(io::File &input_file) const;

    protected:
        virtual bool is_recognized_impl(io::File &input_file) const = 0;

        virtual std::unique_ptr<io::File> decode_impl(
            io::File &input_file) const = 0;
    };

} }
