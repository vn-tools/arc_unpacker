#pragma once

#include "io/file.h"

namespace au {
namespace util {

    class VersionRecognizer final
    {
    public:
        VersionRecognizer();
        ~VersionRecognizer();

        void add_recognizer(
            const int version,
            std::function<bool(io::File &input_file)> func);

        int tell_version(io::File &input_file) const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
