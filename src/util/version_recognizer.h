#pragma once

#include "file.h"

namespace au {
namespace util {

    class VersionRecognizer final
    {
    public:
        VersionRecognizer();
        ~VersionRecognizer();
        void add_recognizer(
            const int version, std::function<bool(File &)> func);
        int tell_version(File &) const;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
