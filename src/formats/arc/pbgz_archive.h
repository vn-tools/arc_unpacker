#ifndef FORMATS_ARC_PBGZ_ARCHIVE
#define FORMATS_ARC_PBGZ_ARCHIVE
#include "formats/archive.h"

class PbgzArchive final : public Archive
{
public:
    PbgzArchive();
    ~PbgzArchive();
    void add_cli_help(ArgParser &) const;
    void parse_cli_options(ArgParser &);
    void unpack_internal(File &, FileSaver &) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
