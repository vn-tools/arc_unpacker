#ifndef FORMATS_TOUHOU_THA1_ARCHIVE_H
#define FORMATS_TOUHOU_THA1_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Touhou
    {
        class Tha1Archive final : public Archive
        {
        public:
            Tha1Archive();
            ~Tha1Archive();
        protected:
            bool is_recognized_internal(File &) const override;
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Priv;
            std::unique_ptr<Priv> p;
        };
    }
}

#endif
