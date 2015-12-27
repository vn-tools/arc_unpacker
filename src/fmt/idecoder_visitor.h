#pragma once

#include "fmt/archive_decoder.h"
#include "fmt/audio_decoder.h"
#include "fmt/file_decoder.h"
#include "fmt/image_decoder.h"

namespace au {
namespace fmt {

    class IDecoderVisitor
    {
    public:
        virtual void visit(const ArchiveDecoder &decoder) = 0;
        virtual void visit(const FileDecoder &decoder) = 0;
        virtual void visit(const ImageDecoder &decoder) = 0;
        virtual void visit(const AudioDecoder &decoder) = 0;
    };

} }
