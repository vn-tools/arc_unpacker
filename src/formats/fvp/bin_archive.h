#ifndef FORMATS_FVP_BIN_ARCHIVE_H
#define FORMATS_FVP_BIN_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Fvp
    {
        class BinArchive final : public Archive
        {
        public:
            BinArchive();
            ~BinArchive();
        protected:
            bool is_recognized_internal(File &) const override;
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
