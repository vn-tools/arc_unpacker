#ifndef FORMATS_ARC_NSA_ARCHIVE
#define FROMATS_ARC_NSA_ARCHIVE
#include "formats/archive.h"

class NsaArchive final : public Archive
{
public:
    NsaArchive();
    ~NsaArchive();
    void unpack_internal(IO &arc_io, OutputFiles &output_files) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
