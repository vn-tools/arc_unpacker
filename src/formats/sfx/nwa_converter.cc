// NWA music
//
// Company:   Key
// Engine:    -
// Extension: .nwa
// Archives:  -
//
// Known games:
// - Clannad
// - Little Busters

#include <cassert>
#include <stdexcept>
#include "formats/sfx/nwa_converter.h"
#include "formats/sound.h"
#include "io.h"
#include "logger.h"

namespace
{
    typedef struct
    {
        uint16_t channel_count;
        uint16_t bits_per_sample;
        uint32_t sample_rate;
        int32_t compression_level;
        uint32_t block_count;
        uint32_t uncompressed_size;
        uint32_t compressed_size;
        uint32_t sample_count;
        uint32_t block_size;
        uint32_t rest_size;
    } NwaHeader;

    bool nwa_validate_header(const NwaHeader &header)
    {
        if (header.compression_level >= 0 || header.compression_level > 5)
        {
            log_error("Unsupported compression level");
            return false;
        }
        else if (header.channel_count != 1 && header.channel_count != 2)
        {
            log_error("Unsupported channel count");
            return false;
        }
        else if (header.bits_per_sample != 8 && header.bits_per_sample != 16)
        {
            log_error("Unsupported bits per sample");
            return false;
        }
        else if (!header.block_count)
        {
            log_error("No blocks found");
            return false;
        }
        else if (!header.compressed_size)
        {
            log_error("No data found");
            return false;
        }
        else if (header.uncompressed_size
            != header.sample_count * header.bits_per_sample / 8)
        {
            log_error("Bad data size");
            return false;
        }
        else if (header.sample_count
            != (header.block_count-1) * header.block_size + header.rest_size)
        {
            log_error("Bad sample count");
            return false;
        }
        return true;
    }

    std::string nwa_read_uncompressed(IO &io, const NwaHeader &header)
    {
        return io.read(header.block_size * header.channel_count);
    }

    std::string nwa_read_compressed(
        __attribute__((unused)) IO &io,
        __attribute__((unused)) const NwaHeader &header)
    {
        throw std::runtime_error("Reading compressed streams is not supported");
    }
}

void NwaConverter::decode_internal(VirtualFile &file) const
{
    NwaHeader header;
    header.channel_count = file.io.read_u16_le();
    header.bits_per_sample = file.io.read_u16_le();
    header.sample_rate = file.io.read_u32_le();
    header.compression_level = (int32_t)file.io.read_u32_le();
    header.block_count = file.io.read_u32_le();
    header.uncompressed_size = file.io.read_u32_le();
    header.compressed_size = file.io.read_u32_le();
    header.sample_count = file.io.read_u32_le();
    header.block_size = file.io.read_u32_le();
    header.rest_size = file.io.read_u32_le();

    std::string samples;

    if (header.compression_level == -1
    || header.block_count == 0
    || header.compressed_size == 0
    || header.block_size == 0
    || header.rest_size == 0)
    {
        samples = nwa_read_uncompressed(file.io, header);
    }
    else
    {
        if (!nwa_validate_header(header))
            throw std::runtime_error("Invalid header, decoding aborted");

        samples = nwa_read_compressed(file.io, header);
    }

    std::unique_ptr<Sound> sound = Sound::from_samples(
        header.channel_count,
        header.bits_per_sample / 8,
        header.sample_rate,
        samples);
    sound->update_file(file);
}
