#ifndef FORMATS_CONVERTER_H
#define FORMATS_CONVERTER_H
#include "arg_parser.h"
#include "virtual_file.h"

class Converter
{
public:
    virtual void add_cli_help(ArgParser &) const;
    virtual void parse_cli_options(ArgParser &);
    virtual void decode_internal(VirtualFile &) const;
    virtual ~Converter();

    bool try_decode(VirtualFile &) const;
    void decode(VirtualFile &) const;
};

#endif
