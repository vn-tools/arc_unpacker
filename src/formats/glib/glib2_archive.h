#ifndef AU_FMT_GLIB_GLIB2_ARCHIVE_H
#define AU_FMT_GLIB_GLIB2_ARCHIVE_H
#include "formats/archive.h"

namespace au {
namespace fmt {
namespace glib {

    class Glib2Archive final : public Archive
    {
    public:
        Glib2Archive();
        ~Glib2Archive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif
