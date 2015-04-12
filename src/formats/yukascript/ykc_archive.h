#ifndef FORMATS_YUKASCRIPT_YKC_ARCHIVE_H
#define FORMATS_YUKASCRIPT_YKC_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace YukaScript
    {
        class YkcArchive final : public Archive
        {
        public:
            YkcArchive();
            ~YkcArchive();
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
