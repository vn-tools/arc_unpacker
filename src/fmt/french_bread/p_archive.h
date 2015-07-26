#ifndef AU_FMT_FRENCH_BREAD_P_ARCHIVE_H
#define AU_FMT_FRENCH_BREAD_P_ARCHIVE_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace french_bread {

    class PArchive final : public Archive
    {
    public:
        PArchive();
        ~PArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif
