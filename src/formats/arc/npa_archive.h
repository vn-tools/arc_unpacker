#ifndef FORMATS_ARC_NPA_ARCHIVE
#define FORMATS_ARC_NPA_ARCHIVE
#include "formats/archive.h"

class NpaArchive final : public Archive
{
public:
    NpaArchive();
    ~NpaArchive();
    void add_cli_help(ArgParser &arg_parser) const override;
    void parse_cli_options(ArgParser &arg_parser) override;
    void unpack_internal(IO &arc_io, OutputFiles &output_files) const override;
private:
    struct Context;
    Context *context;
};

#endif
