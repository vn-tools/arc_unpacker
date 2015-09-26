#pragma once

#include <memory>
#include <vector>
#include "arg_parser.h"
#include "file.h"
#include "file_saver.h"
#include "types.h"

#include "fmt/registry.h"

namespace au {
namespace fmt {

    class INamingStrategy;

    class IDecoder
    {
    public:
        virtual ~IDecoder() { }
        virtual void register_cli_options(ArgParser &) const = 0;
        virtual void parse_cli_options(const ArgParser &) = 0;
        virtual bool is_recognized(File &) const = 0;
        virtual void unpack(File &, FileSaver &, bool) const = 0;
        virtual std::unique_ptr<INamingStrategy> naming_strategy() const = 0;
    };

} }
