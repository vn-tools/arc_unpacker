#ifndef FORMATS_ARC_NPA_ARCHIVE
#define FROMATS_ARC_NPA_ARCHIVE
#include "formats/archive.h"

class NpaArchive final : public Archive
{
public:
    NpaArchive();
    ~NpaArchive();
    void add_cli_help(ArgParser &arg_parser) override;
    void parse_cli_options(ArgParser &arg_parser) override;
    bool unpack_internal(IO *arc_io, OutputFiles &output_files) override;
private:
    struct Context;
    Context *context;
};

#endif
