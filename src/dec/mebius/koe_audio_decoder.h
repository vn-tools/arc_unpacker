#pragma once

#include "dec/base_audio_decoder.h"
#include "plugin_manager.h"

namespace au {
namespace dec {
namespace mebius {

    struct KoePlugin final
    {
        bstr bgm_key;
        bstr koe_key;
        bstr mse_key;
    };

    class KoeAudioDecoder final : public BaseAudioDecoder
    {
    public:
        KoeAudioDecoder();

    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        res::Audio decode_impl(
            const Logger &logger, io::File &input_file) const override;

    public:
        PluginManager<KoePlugin> plugin_manager;
    };

} } }
