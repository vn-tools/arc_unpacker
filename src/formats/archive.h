#ifndef FORMATS_ARCHIVE_H
#define FORMATS_ARCHIVE_H
#include "arg_parser.h"
#include "io.h"
#include "output_files.h"

class Archive
{
public:
    virtual void add_cli_help(ArgParser &);
    virtual void parse_cli_options(ArgParser &);
    virtual bool unpack_internal(IO &, OutputFiles &);
    virtual ~Archive();

    bool unpack(IO &, OutputFiles &);
};

#endif
