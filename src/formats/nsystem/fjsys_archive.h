#ifndef AU_FMT_NSYSTEM_FJSYS_ARCHIVE_H
#define AU_FMT_NSYSTEM_FJSYS_ARCHIVE_H
#include "formats/archive.h"

namespace au {
namespace fmt {
namespace nsystem {

    class FjsysArchive final : public Archive
    {
    public:
        FjsysArchive();
        ~FjsysArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif
