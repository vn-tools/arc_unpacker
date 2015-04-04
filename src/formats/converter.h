#ifndef FORMATS_CONVERTER_H
#define FORMATS_CONVERTER_H
#include "formats/transformer.h"

class Converter : public Transformer
{
public:
    virtual void add_cli_help(ArgParser &) const override;
    virtual void parse_cli_options(const ArgParser &) override;
    virtual FileNamingStrategy get_file_naming_strategy() const override;
    virtual ~Converter();

    std::unique_ptr<File> decode(File &) const;
protected:
    virtual std::unique_ptr<File> decode_internal(File &) const = 0;

protected:
    virtual void unpack_internal(File &, FileSaver &) const final override;
};

#endif
