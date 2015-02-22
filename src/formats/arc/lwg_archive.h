#ifndef FORMATS_ARC_LWG_ARCHIVE
#define FORMATS_ARC_LWG_ARCHIVE
#include "formats/archive.h"

class LwgArchive final : public Archive
{
public:
    LwgArchive();
    ~LwgArchive();
    void add_cli_help(ArgParser &) const override;
    void parse_cli_options(ArgParser &) override;
    void unpack_internal(File &, FileSaver &) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
