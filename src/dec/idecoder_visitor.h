#pragma once

#include "dec/base_archive_decoder.h"
#include "dec/base_audio_decoder.h"
#include "dec/base_file_decoder.h"
#include "dec/base_image_decoder.h"

namespace au {
namespace dec {

    class IDecoderVisitor
    {
    public:
        virtual ~IDecoderVisitor() {}
        virtual void visit(const BaseArchiveDecoder &decoder) = 0;
        virtual void visit(const BaseFileDecoder &decoder) = 0;
        virtual void visit(const BaseImageDecoder &decoder) = 0;
        virtual void visit(const BaseAudioDecoder &decoder) = 0;
    };

} }
