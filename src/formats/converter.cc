#include <cassert>
#include "formats/converter.h"
#include "logger.h"

Converter *converter_create()
{
    Converter *converter = new Converter;
    assert(converter != nullptr);
    converter->data = nullptr;
    converter->add_cli_help = nullptr;
    converter->parse_cli_options = nullptr;
    converter->decode = nullptr;
    converter->cleanup = nullptr;
    return converter;
}

void converter_destroy(Converter *converter)
{
    assert(converter != nullptr);
    if (converter->cleanup != nullptr)
        converter->cleanup(converter);
    delete converter;
}

void converter_parse_cli_options(Converter *converter, ArgParser &arg_parser)
{
    assert(converter != nullptr);
    if (converter->parse_cli_options != nullptr)
        converter->parse_cli_options(converter, arg_parser);
}

void converter_add_cli_help(Converter *converter, ArgParser &arg_parser)
{
    assert(converter != nullptr);
    if (converter->add_cli_help != nullptr)
        converter->add_cli_help(converter, arg_parser);
}

bool converter_decode(Converter *converter, VirtualFile *target_file)
{
    assert(converter != nullptr);
    if (converter->decode == nullptr)
    {
        log_error("Decoding for this format is not supported");
        return false;
    }
    io_seek(target_file->io, 0);
    return converter->decode(converter, target_file);
}

bool converter_try_decode(Converter *converter, VirtualFile *target_file)
{
    return converter_decode(converter, target_file);
}
