#ifndef AU_FMT_ALICE_SOFT_PM_CONVERTER_H
#define AU_FMT_ALICE_SOFT_PM_CONVERTER_H
#include "fmt/converter.h"
#include "util/image.h"

namespace au {
namespace fmt {
namespace alice_soft {

    class PmConverter final : public Converter
    {
    public:
        std::unique_ptr<au::util::Image> decode_to_image(io::IO &io) const;
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    };

} } }

#endif
