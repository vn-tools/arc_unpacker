#ifndef FORMATS_ARCHIVE_H
#define FORMATS_ARCHIVE_H
#include "arg_parser.h"
#include "file.h"
#include "file_saver.h"

class Archive
{
public:
    virtual void add_cli_help(ArgParser &) const;
    virtual void parse_cli_options(ArgParser &);
    virtual void unpack_internal(File &, FileSaver &) const;
    virtual ~Archive();

    bool try_unpack(File &, FileSaver &) const;
    void unpack(File &, FileSaver &) const;
};

#endif
