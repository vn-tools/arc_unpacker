#ifndef FORMATS_ARC_PBG4_ARCHIVE
#define FORMATS_ARC_PBG4_ARCHIVE
#include "formats/archive.h"

class Pbg4Archive final : public Archive
{
public:
    Pbg4Archive();
    ~Pbg4Archive();
    void add_cli_help(ArgParser &) const;
    void parse_cli_options(ArgParser &);
    void unpack_internal(File &, FileSaver &) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
