#include <cassert>
#include "formats/converter.h"
#include "logger.h"

Converter *converter_create()
{
    Converter *converter = new Converter;
    assert(converter != NULL);
    converter->data = NULL;
    converter->add_cli_help = NULL;
    converter->parse_cli_options = NULL;
    converter->decode = NULL;
    converter->cleanup = NULL;
    return converter;
}

void converter_destroy(Converter *converter)
{
    assert(converter != NULL);
    if (converter->cleanup != NULL)
        converter->cleanup(converter);
    delete converter;
}

void converter_parse_cli_options(Converter *converter, ArgParser *arg_parser)
{
    assert(converter != NULL);
    if (converter->parse_cli_options != NULL)
        converter->parse_cli_options(converter, arg_parser);
}

void converter_add_cli_help(Converter *converter, ArgParser *arg_parser)
{
    assert(converter != NULL);
    if (converter->add_cli_help != NULL)
        converter->add_cli_help(converter, arg_parser);
}

bool converter_decode(Converter *converter, VirtualFile *target_file)
{
    assert(converter != NULL);
    if (converter->decode == NULL)
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
