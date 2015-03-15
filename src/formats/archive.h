#ifndef FORMATS_ARCHIVE_H
#define FORMATS_ARCHIVE_H
#include <memory>
#include <vector>
#include "arg_parser.h"
#include "file.h"
#include "file_saver.h"
#include "formats/converter.h"

class Archive
{
public:
    virtual void add_cli_help(ArgParser &) const;
    virtual void parse_cli_options(ArgParser &);
    virtual void unpack_internal(File &, FileSaver &) const;
    virtual ~Archive();

    void register_converter(std::unique_ptr<Converter> converter);
    void run_converters(File &) const;

    bool try_unpack(File &, FileSaver &) const;
    void unpack(File &, FileSaver &) const;
private:
    std::vector<std::unique_ptr<Converter>> converters;
};

#endif
