#ifndef FORMATS_ARC_ARC_ARCHIVE
#define FORMATS_ARC_ARC_ARCHIVE
#include "formats/archive.h"

class ArcArchive final : public Archive
{
public:
    ArcArchive();
    ~ArcArchive();
    void add_cli_help(ArgParser &) const override;
    void parse_cli_options(ArgParser &) override;
    void unpack_internal(VirtualFile &, OutputFiles &) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
