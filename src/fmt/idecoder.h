#pragma once

#include <memory>
#include <vector>
#include "arg_parser.h"
#include "file_saver.h"
#include "fmt/registry.h"
#include "io/file.h"
#include "logger.h"
#include "types.h"

namespace au {
namespace fmt {

    class IDecoder
    {
    public:
        enum class NamingStrategy : u8
        {
            Child = 0,
            Root = 1,
            Sibling = 2,
        };

        virtual ~IDecoder() { }

        virtual void register_cli_options(ArgParser &arg_parser) const = 0;
        virtual void parse_cli_options(const ArgParser &arg_parser) = 0;

        virtual bool is_recognized(io::File &input_file) const = 0;

        virtual void unpack(
            const Logger &logger,
            io::File &input_file,
            const FileSaver &file_saver) const = 0;

        virtual NamingStrategy naming_strategy() const = 0;
    };

    class BaseDecoder : public IDecoder
    {
    public:
        virtual ~BaseDecoder() { }

        virtual void register_cli_options(ArgParser &arg_parser) const override;
        virtual void parse_cli_options(const ArgParser &arg_parser) override;
        virtual bool is_recognized(io::File &input_file) const override;

    protected:
        virtual bool is_recognized_impl(io::File &input_file) const = 0;
    };
} }
