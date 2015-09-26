#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace glib {

    class Glib2Archive final : public ArchiveDecoder
    {
    public:
        Glib2Archive();
        ~Glib2Archive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
