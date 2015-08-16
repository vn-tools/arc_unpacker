#ifndef AU_FMT_TOUHOU_TFBM_CONVERTER_H
#define AU_FMT_TOUHOU_TFBM_CONVERTER_H
#include "fmt/converter.h"
#include "fmt/touhou/palette_map.h"

namespace au {
namespace fmt {
namespace touhou {

    class TfbmConverter : public Converter
    {
    public:
        TfbmConverter();
        ~TfbmConverter();
        void set_palette_map(const PaletteMap &palette_map);
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif
