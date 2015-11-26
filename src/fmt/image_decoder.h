#pragma once

#include "idecoder.h"
#include "pix/grid.h"

namespace au {
namespace fmt {

    class ImageDecoder : public IDecoder
    {
    public:
        virtual ~ImageDecoder();

        virtual void register_cli_options(ArgParser &arg_parser) const override;
        virtual void parse_cli_options(const ArgParser &arg_parser) override;

        bool is_recognized(io::File &input_file) const override;

        void unpack(
            io::File &input_file,
            const FileSaver &file_saver) const override;

        NamingStrategy naming_strategy() const override;

        pix::Grid decode(io::File &input_file) const;

    protected:
        virtual bool is_recognized_impl(io::File &input_file) const = 0;
        virtual pix::Grid decode_impl(io::File &input_file) const = 0;
    };

} }
