#ifndef FORMATS_ARC_NSA_ARCHIVE
#define FORMATS_ARC_NSA_ARCHIVE
#include "formats/archive.h"

class NsaArchive final : public Archive
{
public:
    NsaArchive();
    ~NsaArchive();
    void unpack_internal(VirtualFile &, OutputFiles &) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
