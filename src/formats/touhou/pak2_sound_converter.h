#ifndef FORMATS_TOUHOU_PAK2_SOUND_CONVERTER_H
#define FORMATS_TOUHOU_PAK2_SOUND_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace Touhou
    {
        class Pak2SoundConverter : public Converter
        {
        public:
            void decode_internal(File &) const override;
        };
    }
}

#endif
