#ifndef FORMATS_ARC_P_ARCHIVE
#define FORMATS_ARC_P_ARCHIVE
#include "formats/archive.h"

class PArchive final : public Archive
{
public:
    PArchive();
    ~PArchive();
    void add_cli_help(ArgParser &arg_parser) const override;
    void parse_cli_options(ArgParser &arg_parser) override;
    void unpack_internal(IO &arc_io, OutputFiles &output_files) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
