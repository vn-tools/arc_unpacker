#ifndef AU_FMT_FVP_BIN_ARCHIVE_H
#define AU_FMT_FVP_BIN_ARCHIVE_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace fvp {

    class BinArchive final : public Archive
    {
    public:
        BinArchive();
        ~BinArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif
