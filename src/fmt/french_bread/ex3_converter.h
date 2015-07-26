#ifndef AU_FMT_FRENCH_BREAD_EX3_CONVERTER_H
#define AU_FMT_FRENCH_BREAD_EX3_CONVERTER_H
#include "fmt/converter.h"

namespace au {
namespace fmt {
namespace french_bread {

    class Ex3Converter final : public Converter
    {
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    };

} } }

#endif
