#ifndef AU_FMT_TOUHOU_PBG4_ARCHIVE_H
#define AU_FMT_TOUHOU_PBG4_ARCHIVE_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace touhou {

    class Pbg4Archive final : public Archive
    {
    public:
        Pbg4Archive();
        ~Pbg4Archive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif
