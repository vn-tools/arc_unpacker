#ifndef FORMATS_ARCHIVE_H
#define FORMATS_ARCHIVE_H
#include "arg_parser.h"
#include "output_files.h"
#include "virtual_file.h"

class Archive
{
public:
    virtual void add_cli_help(ArgParser &) const;
    virtual void parse_cli_options(ArgParser &);
    virtual void unpack_internal(VirtualFile &, OutputFiles &) const;
    virtual ~Archive();

    bool try_unpack(VirtualFile &, OutputFiles &) const;
    void unpack(VirtualFile &, OutputFiles &) const;
};

#endif
