#pragma once

#include "dec/wild_bug/wpx/common.h"

namespace au {
namespace dec {
namespace wild_bug {
namespace wpx {

    struct IRetrievalStrategy
    {
        IRetrievalStrategy(io::BaseBitStream &bit_stream) {}
        virtual ~IRetrievalStrategy() {}
        virtual u8 fetch_byte(DecoderContext &, const u8 *) = 0;
    };

    struct RetrievalStrategy1 final : IRetrievalStrategy
    {
        RetrievalStrategy1(io::BaseBitStream &bit_stream, s8 quant_size);
        u8 fetch_byte(DecoderContext &, const u8 *) override;
        std::vector<u8> dict;
        std::vector<u8> table;
        s8 quant_size;
    };

    struct RetrievalStrategy2 final : IRetrievalStrategy
    {
        RetrievalStrategy2(io::BaseBitStream &bit_stream);
        u8 fetch_byte(DecoderContext &, const u8 *) override;
        std::vector<u8> table;
    };

    struct RetrievalStrategy3 final : IRetrievalStrategy
    {
        RetrievalStrategy3(io::BaseBitStream &bit_stream);
        u8 fetch_byte(DecoderContext &, const u8 *) override;
    };

} } } }
