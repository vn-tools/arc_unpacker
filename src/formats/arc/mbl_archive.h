#ifndef FORMATS_ARC_MBL_ARCHIVE
#define FROMATS_ARC_MBL_ARCHIVE
#include "formats/archive.h"

class MblArchive final : public Archive
{
public:
    MblArchive();
    ~MblArchive();
    void add_cli_help(ArgParser &arg_parser) const override;
    void parse_cli_options(ArgParser &arg_parser) override;
    void unpack_internal(IO &arc_io, OutputFiles &output_files) const override;
private:
    struct Context;
    Context *context;
};

#endif
