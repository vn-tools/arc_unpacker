#pragma once

#include <memory>
#include "fmt/kirikiri/xp3_filters/cxdec_settings.h"
#include "fmt/kirikiri/xp3_filter.h"

namespace au {
namespace fmt {
namespace kirikiri {
namespace xp3_filters {

    class Cxdec : public Xp3Filter
    {
    public:
        Cxdec();
        ~Cxdec();
        virtual void decode(File &file, u32 key) const override;
        virtual CxdecSettings get_settings() const = 0;
        void set_arc_path(const std::string &path) override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
