#ifndef FORMATS_ARC_XP3_ARCHIVE
#define FORMATS_ARC_XP3_ARCHIVE
#include "formats/archive.h"

class Xp3Archive final : public Archive
{
public:
    Xp3Archive();
    ~Xp3Archive();
    void add_cli_help(ArgParser &) const override;
    void parse_cli_options(ArgParser &) override;
    void unpack_internal(VirtualFile &, OutputFiles &) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
