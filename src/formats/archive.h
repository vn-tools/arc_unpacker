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

    virtual void unpack(File &file, FileSaver &file_saver) const override;
protected:
    virtual void unpack_internal(File &, FileSaver &) const = 0;
    void add_transformer(Transformer *transformer);

private:
    std::vector<Transformer*> transformers;
};

#endif
