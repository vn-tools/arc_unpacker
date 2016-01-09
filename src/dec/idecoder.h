#pragma once

#include <memory>
#include <vector>
#include "algo/naming_strategies.h"
#include "arg_parser_decorator.h"
#include "io/file.h"

namespace au {
namespace dec {

    class IDecoderVisitor;

    class IDecoder
    {
    public:
        virtual ~IDecoder() {}

        virtual void accept(IDecoderVisitor &visitor) const = 0;

        virtual std::vector<ArgParserDecorator>
            get_arg_parser_decorators() const = 0;

        virtual bool is_recognized(io::File &input_file) const = 0;

        virtual std::vector<std::string> get_linked_formats() const = 0;

        virtual algo::NamingStrategy naming_strategy() const = 0;
    };

} }
