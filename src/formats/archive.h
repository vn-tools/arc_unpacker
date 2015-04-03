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
    virtual ~Archive();

    bool try_unpack(File &, FileSaver &) const;
    void unpack(File &, FileSaver &) const;
    virtual void unpack_internal(File &, FileSaver &) const;

    virtual void add_cli_help(ArgParser &) const;
    virtual void parse_cli_options(const ArgParser &);

protected:
    void add_transformer(Converter *converter);
    void add_transformer(Archive *archive);

private:
    std::vector<Converter*> converters;
    std::vector<Archive*> archives;
};

#endif
