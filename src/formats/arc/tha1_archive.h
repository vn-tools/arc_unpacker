#ifndef FORMATS_ARC_THA1_ARCHIVE
#define FORMATS_ARC_THA1_ARCHIVE
#include "formats/archive.h"

class Tha1Archive final : public Archive
{
public:
    Tha1Archive();
    ~Tha1Archive();
    void add_cli_help(ArgParser &) const;
    void parse_cli_options(ArgParser &);
    void unpack_internal(File &, FileSaver &) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
