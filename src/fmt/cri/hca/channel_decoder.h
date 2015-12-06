#pragma once

#include "fmt/cri/hca/ath_table.h"
#include "fmt/cri/hca/custom_bit_reader.h"

namespace au {
namespace fmt {
namespace cri {
namespace hca {

    class ChannelDecoder final
    {
    public:
        ChannelDecoder(int type, int idx, int count);

        void decode1(
            CustomBitReader &bit_reader,
            unsigned int a,
            int b,
            const AthTable &ath_table);

        void decode2(CustomBitReader &bit_reader);

        void decode3(
            unsigned int a,
            unsigned int b,
            unsigned int c,
            unsigned int d);

        void decode4(
            int index,
            unsigned int a,
            unsigned int b,
            unsigned int c);

        void decode5(int index);

        float wave[8][0x80];

    private:
        int type;
        unsigned int count;
        u8 scale[0x80];
        u8 value[0x80];
        u8 value2[8];
        u8 *value3;
        float block[0x80];
        float base[0x80];
        float wav1[0x80];
        float wav2[0x80];
        float wav3[0x80];
    };

} } } }
