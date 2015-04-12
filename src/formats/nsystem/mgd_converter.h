#ifndef FORMATS_NSYSTEM_MGD_CONVERTER_H
#define FORMATS_NSYSTEM_MGD_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace NSystem
    {
        class MgdConverter final : public Converter
        {
        protected:
            bool is_recognized_internal(File &) const override;
            std::unique_ptr<File> decode_internal(File &) const override;
        };
    }
}

#endif
