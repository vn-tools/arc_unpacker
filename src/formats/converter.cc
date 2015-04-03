#include <cassert>
#include <stdexcept>
#include "formats/converter.h"
#include "logger.h"

void Converter::add_cli_help(ArgParser &) const
{
}

void Converter::parse_cli_options(const ArgParser &)
{
}

void Converter::decode_internal(File &) const
{
    throw std::runtime_error("Decoding is not supported");
}

void Converter::decode(File &target_file) const
{
    target_file.io.seek(0);
    decode_internal(target_file);
}

bool Converter::try_decode(File &target_file) const
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
