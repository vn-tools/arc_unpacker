#include <stdlib.h>
#include "assert_ex.h"
#include "io.h"
#include "sound.h"
#include "logger.h"
#include "formats/sfx/nwa_converter.h"

// Converts NWA to WAV and vice versa.
// Seen in Key games:
// - Clannad
// - Little Busters

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

static bool nwa_validate_header(NwaHeader *header)
{
    if (header->compression_level >= 0 || header->compression_level > 5)
    {
        log_error("Unsupported compression level");
        return false;
    }
    else if (header->channel_count != 1 && header->channel_count != 2)
    {
        log_error("Unsupported channel count");
        return false;
    }
    else if (header->bits_per_sample != 8 && header->bits_per_sample != 16)
    {
        log_error("Unsupported bits per sample");
        return false;
    }
    else if (!header->block_count)
    {
        log_error("No blocks found");
        return false;
    }
    else if (!header->compressed_size)
    {
        log_error("No data found");
        return false;
    }
    else if (header->uncompressed_size
        != header->sample_count * header->bits_per_sample / 8)
    {
        log_error("Bad data size");
        return false;
    }
    else if (header->sample_count
        != (header->block_count - 1) * header->block_size + header->rest_size)
    {
        log_error("Bad sample count");
        return false;
    }
    return true;
}

static bool nwa_read_uncompressed(
    IO *io,
    NwaHeader *header,
    char **samples,
    size_t *sample_count)
{
    assert_not_null(io);
    assert_not_null(header);
    assert_not_null(samples);
    assert_not_null(sample_count);
    *sample_count = header->block_size * header->channel_count;
    *samples = io_read_string(io, *sample_count);
    if (*samples == NULL)
        return false;
    return true;
}

static bool nwa_read_compressed(
    IO *io,
    NwaHeader *header,
    __attribute__((unused)) char **samples,
    __attribute__((unused)) size_t *sample_count)
{
    assert_not_null(io);
    assert_not_null(header);
    log_error("Reading compressed streams is not supported");
    return false;
}

static bool nwa_decode(Converter *converter, VirtualFile *file)
{
    assert_not_null(converter);
    assert_not_null(file);

    IO *io = io_create_from_buffer(vf_get_data(file), vf_get_size(file));
    assert_not_null(io);

    NwaHeader header;
    header.channel_count = io_read_u16_le(io);
    header.bits_per_sample = io_read_u16_le(io);
    header.sample_rate = io_read_u32_le(io);
    header.compression_level = (int32_t)io_read_u32_le(io);
    header.block_count = io_read_u32_le(io);
    header.uncompressed_size = io_read_u32_le(io);
    header.compressed_size = io_read_u32_le(io);
    header.sample_count = io_read_u32_le(io);
    header.block_size = io_read_u32_le(io);
    header.rest_size = io_read_u32_le(io);

    char *output_samples = NULL;
    size_t output_sample_count;
    bool result;

    if (header.compression_level == -1
        || header.block_count == 0
        || header.compressed_size == 0
        || header.block_size == 0
        || header.rest_size == 0)
    {
        result = nwa_read_uncompressed(
            io,
            &header,
            &output_samples,
            &output_sample_count);
    }
    else
    {
        if (!nwa_validate_header(&header))
        {
            log_error("Invalid header, decoding aborted");
            io_destroy(io);
            return false;
        }

        result = nwa_read_compressed(
            io,
            &header,
            &output_samples,
            &output_sample_count);
    }

    if (result)
    {
        Sound *sound = sound_create_from_samples(
            header.channel_count,
            header.bits_per_sample / 8,
            header.sample_rate,
            output_samples,
            output_sample_count);
        sound_update_file(sound, file);
        sound_destroy(sound);
    }

    io_destroy(io);
    free(output_samples);
    return true;
}

Converter *nwa_converter_create()
{
    Converter *converter = converter_create();
    converter->decode = &nwa_decode;
    return converter;
}
