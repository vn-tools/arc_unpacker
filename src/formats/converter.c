#include <stdlib.h>
#include "assert_ex.h"
#include "formats/converter.h"
#include "logger.h"

Converter *converter_create()
{
    Converter *converter = (Converter*)malloc(sizeof(Converter));
    assert_not_null(converter);
    converter->data = NULL;
    converter->add_cli_help = NULL;
    converter->parse_cli_options = NULL;
    converter->decode = NULL;
    converter->cleanup = NULL;
    return converter;
}

void converter_destroy(Converter *converter)
{
    assert_not_null(converter);
    if (converter->cleanup != NULL)
        converter->cleanup(converter);
    free(converter);
}

void converter_parse_cli_options(Converter *converter, ArgParser *arg_parser)
{
    assert_not_null(converter);
    if (converter->parse_cli_options != NULL)
        converter->parse_cli_options(converter, arg_parser);
}

void converter_add_cli_help(Converter *converter, ArgParser *arg_parser)
{
    assert_not_null(converter);
    if (converter->add_cli_help != NULL)
        converter->add_cli_help(converter, arg_parser);
}

bool converter_decode(Converter *converter, VirtualFile *target_file)
{
    assert_not_null(converter);
    if (converter->decode == NULL)
    {
        log_error("Decoding for this format is not supported");
        return false;
    }
    return converter->decode(converter, target_file);
}

bool converter_try_decode(Converter *converter, VirtualFile *target_file)
{
    log_save();
    log_silence();
    bool result = converter_decode(converter, target_file);
    log_restore();
    return result;
}
