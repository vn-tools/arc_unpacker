#ifndef AU_FMT_TOUHOU_PAK2_IMAGE_CONVERTER_H
#define AU_FMT_TOUHOU_PAK2_IMAGE_CONVERTER_H
#include <string>
#include "fmt/converter.h"
#include "fmt/touhou/palette_map.h"

namespace au {
namespace fmt {
namespace touhou {

    class Pak2ImageConverter : public Converter
    {
    public:
        Pak2ImageConverter();
        ~Pak2ImageConverter();
        void add_palette(const std::string &name, const bstr &palette_data);
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif
