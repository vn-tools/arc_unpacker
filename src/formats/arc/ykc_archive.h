#ifndef FORMATS_ARC_YKC_ARCHIVE
#define FROMATS_ARC_YKC_ARCHIVE
#include "formats/archive.h"

class YkcArchive final : public Archive
{
public:
    YkcArchive();
    ~YkcArchive();
    void add_cli_help(ArgParser &arg_parser) const override;
    void parse_cli_options(ArgParser &arg_parser) override;
    void unpack_internal(IO &arc_io, OutputFiles &output_files) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif

