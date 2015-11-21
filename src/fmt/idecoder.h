#pragma once

#include <memory>
#include <vector>
#include "arg_parser.h"
#include "file_saver.h"
#include "io/file.h"
#include "types.h"

#include "fmt/registry.h"

namespace au {
namespace fmt {

    class INamingStrategy;

    class IDecoder
    {
    public:
        virtual ~IDecoder() { }

        virtual void register_cli_options(ArgParser &arg_parser) const = 0;
        virtual void parse_cli_options(const ArgParser &arg_parser) = 0;

        virtual bool is_recognized(io::File &input_file) const = 0;

        virtual void unpack(
            io::File &input_file,
            const FileSaver &file_saver) const = 0;

        virtual std::unique_ptr<INamingStrategy> naming_strategy() const = 0;
    };

} }
