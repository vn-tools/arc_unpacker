#ifndef FORMATS_ARC_PBG3_ARCHIVE
#define FORMATS_ARC_PBG3_ARCHIVE
#include "formats/archive.h"

class Pbg3Archive final : public Archive
{
public:
    Pbg3Archive();
    ~Pbg3Archive();
    void add_cli_help(ArgParser &) const;
    void parse_cli_options(ArgParser &);
    void unpack_internal(File &, FileSaver &) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
