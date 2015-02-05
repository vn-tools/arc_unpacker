#ifndef FORMATS_CONVERTER_H
#define FORMATS_CONVERTER_H
#include "arg_parser.h"
#include "virtual_file.h"

class Converter
{
public:
    virtual void add_cli_help(ArgParser &);
    virtual void parse_cli_options(ArgParser &);
    virtual bool decode_internal(VirtualFile *);
    virtual ~Converter();

    bool try_decode(VirtualFile *);
    bool decode(VirtualFile *);
};

Converter *converter_create();

#endif
