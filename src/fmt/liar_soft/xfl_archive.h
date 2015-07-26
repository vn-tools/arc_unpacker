#ifndef AU_FMT_LIAR_SOFT_XFL_ARCHIVE_H
#define AU_FMT_LIAR_SOFT_XFL_ARCHIVE_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace liar_soft {

    class XflArchive final : public Archive
    {
    public:
        XflArchive();
        ~XflArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif
