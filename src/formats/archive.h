#ifndef FORMATS_ARCHIVE_H
#define FORMATS_ARCHIVE_H
#include "formats/transformer.h"

class Archive : public Transformer
{
public:
    virtual void add_cli_help(ArgParser &) const override;
    virtual void parse_cli_options(const ArgParser &) override;
    virtual FileNamingStrategy get_file_naming_strategy() const override;
    virtual ~Archive();

protected:
    void unpack_internal(File &, FileSaver &) const override = 0;
};

#endif
