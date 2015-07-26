#ifndef AU_FMT_TOUHOU_PAK1_ARCHIVE_H
#define AU_FMT_TOUHOU_PAK1_ARCHIVE_H
#include "formats/archive.h"

namespace au {
namespace fmt {
namespace touhou {

    class Pak1Archive final : public Archive
    {
    public:
        Pak1Archive();
        ~Pak1Archive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif
