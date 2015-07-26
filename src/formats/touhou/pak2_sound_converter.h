#ifndef AU_FMT_TOUHOU_PAK2_SOUND_CONVERTER_H
#define AU_FMT_TOUHOU_PAK2_SOUND_CONVERTER_H
#include "formats/converter.h"

namespace au {
namespace fmt {
namespace touhou {

    class Pak2SoundConverter : public Converter
    {
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    };

} } }

#endif
