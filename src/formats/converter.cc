#include <cassert>
#include <stdexcept>
#include "formats/converter.h"
#include "logger.h"

void Converter::add_cli_help(
    __attribute__((unused)) ArgParser &arg_parser) const
{
}

void Converter::parse_cli_options(__attribute__((unused)) ArgParser &arg_parser)
{
}

void Converter::decode_internal(__attribute__((unused)) VirtualFile &) const
{
    throw std::runtime_error("Decoding is not supported");
}

void Converter::decode(VirtualFile &target_file) const
{
    target_file.io.seek(0);
    decode_internal(target_file);
}

bool Converter::try_decode(VirtualFile &target_file) const
{
    try
    {
        decode(target_file);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

Converter::~Converter()
{
}
