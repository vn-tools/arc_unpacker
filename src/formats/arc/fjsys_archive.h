#ifndef FORMATS_ARC_FJSYS_ARCHIVE
#define FORMATS_ARC_FJSYS_ARCHIVE
#include "formats/archive.h"

class FjsysArchive final : public Archive
{
public:
    FjsysArchive();
    ~FjsysArchive();
    void add_cli_help(ArgParser &) const override;
    void parse_cli_options(ArgParser &) override;
    void unpack_internal(File &, FileSaver &) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
