#ifndef AU_FMT_BGI_ARC_ARCHIVE_H
#define AU_FMT_BGI_ARC_ARCHIVE_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace bgi {

    class ArcArchive final : public Archive
    {
    public:
        ArcArchive();
        ~ArcArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif
