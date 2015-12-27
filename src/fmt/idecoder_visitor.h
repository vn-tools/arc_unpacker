#pragma once

#include "fmt/base_archive_decoder.h"
#include "fmt/base_audio_decoder.h"
#include "fmt/base_file_decoder.h"
#include "fmt/base_image_decoder.h"

namespace au {
namespace fmt {

    class IDecoderVisitor
    {
    public:
        virtual void visit(const BaseArchiveDecoder &decoder) = 0;
        virtual void visit(const BaseFileDecoder &decoder) = 0;
        virtual void visit(const BaseImageDecoder &decoder) = 0;
        virtual void visit(const BaseAudioDecoder &decoder) = 0;
    };

} }
