#ifndef AU_FMT_TOUHOU_TFBM_CONVERTER_H
#define AU_FMT_TOUHOU_TFBM_CONVERTER_H
#include <string>
#include "fmt/converter.h"

namespace au {
namespace fmt {
namespace touhou {

    class TfbmConverter : public Converter
    {
    public:
        TfbmConverter();
        ~TfbmConverter();
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
