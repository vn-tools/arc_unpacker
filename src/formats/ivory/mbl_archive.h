#ifndef AU_FMT_IVORY_MBL_ARCHIVE_H
#define AU_FMT_IVORY_MBL_ARCHIVE_H
#include "formats/archive.h"

namespace au {
namespace fmt {
namespace ivory {

    class MblArchive final : public Archive
    {
    public:
        MblArchive();
        ~MblArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif
