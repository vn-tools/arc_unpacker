#pragma once

#include "idecoder.h"

namespace au {
namespace fmt {

    class FileDecoder : public BaseDecoder
    {
    public:
        virtual ~FileDecoder();

        void unpack(
            io::File &input_file,
            const FileSaver &file_saver) const override;

        NamingStrategy naming_strategy() const override;

        std::unique_ptr<io::File> decode(io::File &input_file) const;

    protected:
        virtual std::unique_ptr<io::File> decode_impl(
            io::File &input_file) const = 0;
    };

} }
