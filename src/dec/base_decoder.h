#pragma once

#include <vector>
#include "dec/idecoder.h"
#include "dec/registry.h" // for child decoders

namespace au {
namespace dec {

    class BaseDecoder
        : public IDecoder, public std::enable_shared_from_this<IDecoder>
    {
    public:
        virtual ~BaseDecoder() {}

        virtual void register_cli_options(ArgParser &arg_parser) const override;
        virtual void parse_cli_options(const ArgParser &arg_parser) override;
        virtual bool is_recognized(io::File &input_file) const override;
        virtual std::vector<std::string> get_linked_formats() const override;

    protected:
        virtual bool is_recognized_impl(io::File &input_file) const = 0;
    };

} }
