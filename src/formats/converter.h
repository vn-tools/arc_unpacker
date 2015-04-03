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

    bool try_decode(File &) const;
    void decode(File &) const;

protected:
    virtual void unpack_internal(File &, FileSaver &) const final override;
    virtual void decode_internal(File &) const;
};

#endif
