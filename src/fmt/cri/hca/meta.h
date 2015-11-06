#pragma once

#include <memory>
#include "types.h"

namespace au {
namespace fmt {
namespace cri {
namespace hca {

    struct HcaChunk final
    {
        u16 version;
        size_t data_offset;
    };

    struct FmtChunk final
    {
        size_t channel_count;
        size_t sample_rate;
        size_t block_count;
    };

    struct DecChunk final
    {
        size_t block_size;
        u32 unk[4];
        u8 count[2];
        bool enable_count2;
    };

    struct CompChunk final
    {
        size_t block_size;
        u8 unk[8];
    };

    struct VbrChunk final
    {
        u16 unk[2];
    };

    struct AthChunk final
    {
        u16 type;
    };

    struct LoopChunk final
    {
        size_t start;
        size_t end;
        size_t repetitions;
        u16 unk;
        bool enabled;
    };

    struct CiphChunk final
    {
        u16 type;
    };

    struct RvaChunk final
    {
        float volume;
    };

    struct CommChunk final
    {
        bstr text;
    };

    struct Meta final
    {
        std::unique_ptr<HcaChunk> hca;
        std::unique_ptr<FmtChunk> fmt;
        std::unique_ptr<DecChunk> dec;
        std::unique_ptr<CompChunk> comp;
        std::unique_ptr<VbrChunk> vbr;
        std::unique_ptr<AthChunk> ath;
        std::unique_ptr<LoopChunk> loop;
        std::unique_ptr<CiphChunk> ciph;
        std::unique_ptr<RvaChunk> rva;
        std::unique_ptr<CommChunk> comm;
    };

    Meta read_meta(const bstr &input);

} } } }
