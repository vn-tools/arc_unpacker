#ifndef AU_FMT_QLIE_PACK_ARCHIVE_H
#define AU_FMT_QLIE_PACK_ARCHIVE_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace qlie {

    class PackArchive final : public Archive
    {
    public:
        PackArchive();
        ~PackArchive();
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
