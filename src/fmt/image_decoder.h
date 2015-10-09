#pragma once

#include "idecoder.h"
#include "pix/grid.h"

namespace au {
namespace fmt {

    class ImageDecoder : public IDecoder
    {
    public:
        virtual ~ImageDecoder();
        virtual void register_cli_options(ArgParser &) const override;
        virtual void parse_cli_options(const ArgParser &) override;
        virtual bool is_recognized(File &) const override;
        virtual void unpack(File &, FileSaver &) const override;
        virtual std::unique_ptr<INamingStrategy> naming_strategy()
            const override;

        pix::Grid decode(File &) const;

    protected:
        virtual bool is_recognized_impl(File &) const = 0;
        virtual pix::Grid decode_impl(File &) const = 0;
    };

} }
