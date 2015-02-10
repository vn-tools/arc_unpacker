#ifndef FORMATS_ARC_XP3_ARCHIVE
#define FORMATS_ARC_XP3_ARCHIVE
#include "formats/archive.h"

class Xp3Archive final : public Archive
{
public:
    Xp3Archive();
    ~Xp3Archive();
    void add_cli_help(ArgParser &arg_parser) const override;
    void parse_cli_options(ArgParser &arg_parser) override;
    void unpack_internal(IO &arc_io, OutputFiles &output_files) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
