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

        std::vector<ArgParserDecorator>
            get_arg_parser_decorators() const override;

        virtual bool is_recognized(io::File &input_file) const override;

        virtual std::vector<std::string> get_linked_formats() const override;

    protected:
        void add_arg_parser_decorator(const ArgParserDecorator &decorator);

        void add_arg_parser_decorator(
            const std::function<void(ArgParser &)> register_callback,
            const std::function<void(const ArgParser &)> parse_callback);

        virtual bool is_recognized_impl(io::File &input_file) const = 0;

    private:
        std::vector<ArgParserDecorator> arg_parser_decorators;
    };

} }
