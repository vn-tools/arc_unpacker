#pragma once

#include <memory>
#include <vector>
#include "arg_parser.h"
#include "io/file.h"
#include "naming_strategies.h"

namespace au {
namespace dec {

    class IDecoderVisitor;

    class IDecoder
    {
    public:
        virtual ~IDecoder() {}

        virtual void accept(IDecoderVisitor &visitor) const = 0;

        virtual void register_cli_options(ArgParser &arg_parser) const = 0;

        virtual void parse_cli_options(const ArgParser &arg_parser) = 0;

        virtual bool is_recognized(io::File &input_file) const = 0;

        virtual std::vector<std::string> get_linked_formats() const = 0;

        virtual NamingStrategy naming_strategy() const = 0;
    };

} }
