#ifndef FORMATS_ARC_XP3_ARCHIVE_XP3_FILTER_CXDEC
#define FORMATS_ARC_XP3_ARCHIVE_XP3_FILTER_CXDEC
#include "formats/arc/xp3_archive/xp3_filter.h"
#include "formats/arc/xp3_archive/xp3_filter_cxdec_settings.h"

class Xp3FilterCxdec final : public Xp3Filter
{
public:
    Xp3FilterCxdec(Xp3FilterCxdecSettings &);
    ~Xp3FilterCxdec();
    virtual void decode(VirtualFile &file, uint32_t key) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
