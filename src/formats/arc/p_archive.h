#ifndef FORMATS_ARC_P_ARCHIVE
#define FORMATS_ARC_P_ARCHIVE
#include "formats/archive.h"

class PArchive final : public Archive
{
public:
    PArchive();
    ~PArchive();
    void add_cli_help(ArgParser &) const override;
    void parse_cli_options(ArgParser &) override;
    void unpack_internal(File &, FileSaver &) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
