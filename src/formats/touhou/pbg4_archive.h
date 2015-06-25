#ifndef FORMATS_TOUHOU_PBG4_ARCHIVE_H
#define FORMATS_TOUHOU_PBG4_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Touhou
    {
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
    }
}

#endif
