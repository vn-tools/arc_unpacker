#ifndef AU_FMT_RIDDLE_SOFT_CMP_CONVERTER_H
#define AU_FMT_RIDDLE_SOFT_CMP_CONVERTER_H
#include "fmt/converter.h"

namespace au {
namespace fmt {
namespace riddle_soft {

    class CmpConverter final : public Converter
    {
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    };

} } }

#endif
