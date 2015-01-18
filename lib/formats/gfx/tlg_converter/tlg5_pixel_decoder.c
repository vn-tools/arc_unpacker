#include <ruby.h>
#include "lzss_compressor.h"

typedef struct {
    unsigned char channel_count;
    unsigned long image_width;
    unsigned long image_height;
    unsigned long block_height;
} Tlg5Header;

typedef struct {
    unsigned char mark;
    unsigned long block_size;
    unsigned char *block_data;
} Tlg5BlockInfo;

void tlg5_block_info_destroy(Tlg5BlockInfo *info) {
    free(info->block_data);
}

void tlg5_block_info_read(Tlg5BlockInfo *info, unsigned char **_input) {
    unsigned char *input = *_input;

    info->mark = *input ++;

    info->block_size = *(unsigned long*)input;
    input += 4;

    info->block_data = (unsigned char*) malloc(info->block_size);
    if (!info->block_data) {
        rb_raise(rb_eRuntimeError, "Failed to allocate memory");
        return;
    }

    int i;
    unsigned char *tmp = info->block_data;
    for (i = 0; i < info->block_size; i ++)
        *tmp ++ = *input ++;

    *_input = input;
}

void tlg5_block_info_decompress(
    Tlg5BlockInfo *info,
    LzssState *lzss_state,
    Tlg5Header *header) {

    int new_data_size = header->image_width * header->block_height;
    unsigned char *new_data = (unsigned char*) malloc(new_data_size);
    if (!new_data) {
        rb_raise(rb_eRuntimeError, "Failed to allocate memory");
        return;
    }

    lzss_state_decompress(
        lzss_state,
        info->block_data,
        info->block_size,
        new_data,
        new_data_size);

    free(info->block_data);
    info->block_data = new_data;
    info->block_size = new_data_size;
}

void load_pixel_block_row(
    unsigned char *zero_line,
    unsigned char *output,
    Tlg5BlockInfo *channel_data,
    Tlg5Header *header,
    int block_y) {

    unsigned long max_y = block_y + header->block_height;
    if (max_y > header->image_height)
        max_y = header->image_height;
    int use_alpha = header->channel_count == 4;

    int y, x;
    unsigned char *current_line = &output[block_y * header->image_width * 4];
    unsigned char *previous_line = block_y == 0
        ? zero_line
        : &output[(block_y - 1) * header->image_width * 4];

    for (y = block_y; y < max_y; y ++) {
        unsigned char prev_red = 0;
        unsigned char prev_green = 0;
        unsigned char prev_blue = 0;
        unsigned char prev_alpha = 0;

        int block_y_shift = (y - block_y) * header->image_width;
        unsigned char *current_line_start = current_line;
        for (x = 0; x < header->image_width; x ++) {
            unsigned char red = channel_data[2].block_data[block_y_shift + x];
            unsigned char green = channel_data[1].block_data[block_y_shift + x];
            unsigned char blue = channel_data[0].block_data[block_y_shift + x];
            unsigned char alpha = use_alpha ? channel_data[3].block_data[block_y_shift + x] : 0;

            red += green;
            blue += green;

            prev_red += red;
            prev_green += green;
            prev_blue += blue;
            prev_alpha += alpha;

            unsigned char output_red = prev_red;
            unsigned char output_green = prev_green;
            unsigned char output_blue = prev_blue;
            unsigned char output_alpha = prev_alpha;

            output_red += *previous_line ++;
            output_green += *previous_line ++;
            output_blue += *previous_line ++;
            output_alpha += *previous_line ++;

            if (!use_alpha)
                output_alpha = 0xff;

            *current_line ++ = output_red;
            *current_line ++ = output_green;
            *current_line ++ = output_blue;
            *current_line ++ = output_alpha;
        }
        previous_line = current_line_start;
    }
}

void read_pixels(unsigned char *input, unsigned char *output, Tlg5Header *header) {
    // ignore block sizes
    size_t block_count = (header->image_height - 1) / header->block_height + 1;
    input += 4 * block_count;

    int channel, y;
    LzssState lzss_state;
    lzss_state_init(&lzss_state);

    unsigned char *zero_line = (unsigned char*) malloc(header->image_width * 4);
    memset(zero_line, 0, header->image_width * 4);
    memset(output, 0, header->image_width * header->image_height * 4);

    for (y = 0; y < header->image_height; y += header->block_height) {
        Tlg5BlockInfo *channel_data = (Tlg5BlockInfo*)malloc(sizeof(Tlg5BlockInfo) * header->channel_count);

        for (channel = 0; channel < header->channel_count; channel ++) {
            tlg5_block_info_read(&channel_data[channel], &input);
            if (!channel_data[channel].mark) {
                tlg5_block_info_decompress(&channel_data[channel], &lzss_state, header);
            }
        }

        load_pixel_block_row(zero_line, output, channel_data, header, y);
        for (channel = 0; channel < header->channel_count; channel ++) {
            tlg5_block_info_destroy(&channel_data[channel]);
        }
        free(channel_data);
    }
    free(zero_line);
}

static VALUE decode_tlg5_pixels(
    VALUE _self,
    VALUE _header,
    VALUE _input) {

    size_t input_size = RSTRING_LEN(_input);
    unsigned char *input = RSTRING_PTR(_input);

    Tlg5Header header;
    header.channel_count = FIX2INT(rb_hash_aref(_header, ID2SYM(rb_intern("channel_count"))));
    header.image_width = FIX2INT(rb_hash_aref(_header, ID2SYM(rb_intern("image_width"))));
    header.image_height = FIX2INT(rb_hash_aref(_header, ID2SYM(rb_intern("image_height"))));
    header.block_height = FIX2INT(rb_hash_aref(_header, ID2SYM(rb_intern("block_height"))));
    if (header.channel_count != 3 && header.channel_count != 4) {
        rb_raise(rb_eRuntimeError, "Unsupported channel count");
    }

    size_t output_size = header.image_width * header.image_height * 4;
    unsigned char *output = (unsigned char*)malloc(output_size);

    read_pixels(input, output, &header);

    VALUE ret = rb_str_new((char*) output, output_size);
    free(output);
    return ret;
}

void Init_tlg5_pixel_decoder() {
    rb_define_global_function("decode_tlg5_pixels", decode_tlg5_pixels, 2);
}
