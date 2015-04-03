#ifndef FORMATS_CONVERTER_H
#define FORMATS_CONVERTER_H
#include "arg_parser.h"
#include "file.h"

class Converter
{
public:
    virtual void add_cli_help(ArgParser &) const;
    virtual void parse_cli_options(const ArgParser &);
    virtual void decode_internal(File &) const;
    virtual ~Converter();

    bool try_decode(File &) const;
    void decode(File &) const;
};

#endif
