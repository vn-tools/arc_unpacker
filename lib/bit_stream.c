#include <stdlib.h>

typedef struct {
    unsigned char *buffer;
    size_t buffer_size;
    unsigned int mask;
    unsigned char value;
} bit_stream;

void bit_stream_init(
    bit_stream *bs,
    unsigned char *buffer,
    size_t buffer_size) {

    if (bs == NULL)
        return;

    bs->buffer = buffer;
    bs->buffer_size = buffer_size;

    bs->mask = 0x100;
    bs->value = bs->buffer_size > 0 ? *buffer : 0;
    bs->buffer_size --;
    bs->buffer ++;
}

unsigned int __bit_stream_get(bit_stream *bs) {
    if (bs == NULL)
        return 0;

    bs->mask >>= 1;
    if (bs->mask == 0x00)
    {
        if (bs->buffer_size == 0)
            return 0;

        bs->mask = 0x80;
        bs->value = bs->buffer_size > 0 ? *bs->buffer : 0;
        bs->buffer_size --;
        bs->buffer ++;
    }

    return (bs->value & bs->mask) ? 1 : 0;
}

unsigned int bit_stream_get(bit_stream *bs, int n) {
    unsigned int value;

    if (bs == NULL)
        return 0;

    if (n < 0 || n > 32)
        return 0;

    for (value = 0; n > 0; n--)
    {
        value <<= 1;
        if (__bit_stream_get(bs))
            value |= 1;
    }

    return value;
}
