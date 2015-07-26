#ifndef AU_FMT_KIRIKIRI_XP3_ARCHIVE_H
#define AU_FMT_KIRIKIRI_XP3_ARCHIVE_H
#include "formats/archive.h"

namespace au {
namespace fmt {
namespace kirikiri {

    class Xp3Archive final : public Archive
    {
    public:
        Xp3Archive();
        ~Xp3Archive();
        void add_cli_help(ArgParser &) const override;
        void parse_cli_options(const ArgParser &) override;
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif
