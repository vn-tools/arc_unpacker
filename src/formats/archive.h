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

    void add_transformer(std::shared_ptr<Converter> converter);

    bool try_unpack(File &, FileSaver &) const;
    void unpack(File &, FileSaver &) const;
private:
    std::vector<std::shared_ptr<Converter>> transformers;
};

#endif
