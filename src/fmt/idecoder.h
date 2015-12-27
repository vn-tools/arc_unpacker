#pragma once

#include <memory>
#include <vector>
#include "arg_parser.h"
#include "fmt/registry.h" // for child decoders
#include "io/file.h"
#include "logger.h"
#include "types.h"

namespace au {
namespace fmt {

    class IDecoderVisitor;

    class IDecoder
    {
    public:
        enum class NamingStrategy : u8
        {
            Child = 0,
            Root = 1,
            Sibling = 2,
            FlatSibling = 3,
        };

        virtual ~IDecoder() { }

        virtual void register_cli_options(ArgParser &arg_parser) const = 0;
        virtual void parse_cli_options(const ArgParser &arg_parser) = 0;

        virtual bool is_recognized(io::File &input_file) const = 0;

        virtual void accept(IDecoderVisitor &visitor) const = 0;

        virtual std::vector<std::string> get_linked_formats() const = 0;

        virtual NamingStrategy naming_strategy() const = 0;
    };

    class BaseDecoder
        : public IDecoder, public std::enable_shared_from_this<IDecoder>
    {
    public:
        virtual ~BaseDecoder() { }

        virtual void register_cli_options(ArgParser &arg_parser) const override;
        virtual void parse_cli_options(const ArgParser &arg_parser) override;
        virtual bool is_recognized(io::File &input_file) const override;
        virtual std::vector<std::string> get_linked_formats() const override;

    protected:
        virtual bool is_recognized_impl(io::File &input_file) const = 0;
    };

    io::path decorate_path(
        const IDecoder::NamingStrategy strategy,
        const io::path &parent_name,
        const io::path &current_name);

} }
