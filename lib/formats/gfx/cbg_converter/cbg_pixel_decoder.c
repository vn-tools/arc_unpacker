#include <ruby.h>
#include <stdlib.h>
#include <string.h>

struct CbgNodeInfo {
    unsigned char valid;
    unsigned long frequency;
    int left_node;
    int right_node;
};

static unsigned char *allocate(size_t length) {
    unsigned char *result = (unsigned char*) malloc(length);
    if (!result) {
        rb_raise(rb_eRuntimeError, "Failed to allocate memory");
        return NULL;
    }
    return result;
}

static void *read(unsigned char **input, size_t size) {
    void *result = *input;
    (*input) += size;
    return result;
}

static unsigned long read_ulong(unsigned char **input) {
    return *(unsigned long*)read(input, 4);
}

static unsigned long cbg_get_key(unsigned long *pkey) {
    unsigned long key = *pkey;
    unsigned long tmp1 = 0x4e35 * (key & 0xffff);
    unsigned long tmp2 = 0x4e35 * (key >> 16);
    unsigned long tmp = 0x15a * key + tmp2 + (tmp1 >> 16);
    *pkey = (tmp << 16) + (tmp1 & 0xffff) + 1;
    return tmp & 0x7fff;
}

static void cbg_decrypt(
    unsigned char *input,
    unsigned long decrypt_size,
    unsigned long key) {

    unsigned long i;
    for (i = 0; i < decrypt_size; i ++) {
        *input ++ -= (unsigned char) cbg_get_key(&key);
    }
}

static unsigned long cbg_read_variable_data(
    unsigned char **input,
    unsigned char *input_guardian) {

    unsigned char current;
    unsigned long result = 0;
    unsigned long shift = 0;
    do {
        current = **input;
        (*input) ++;
        if (*input > input_guardian) {
            rb_raise(rb_eRuntimeError, "Reading beyond file");
        }
        result |= (current & 0x7f) << shift;
        shift += 7;
    } while (current & 0x80);
    return result;
}

static void cbg_read_frequency_table(
    unsigned char **input,
    unsigned long raw_data_size,
    unsigned long key,
    unsigned long frequency_table[]) {

    unsigned char *raw_data = allocate(raw_data_size);
    memcpy(raw_data, *input, raw_data_size);
    *input += raw_data_size;

    cbg_decrypt(raw_data, raw_data_size, key);

    int i;
    unsigned char *raw_data_ptr = raw_data;
    for (i = 0; i < 256; i ++) {
        frequency_table[i] = cbg_read_variable_data(&raw_data_ptr, raw_data + raw_data_size);
    }

    free(raw_data);
}

static int cbg_read_node_info(
    unsigned long frequency_table[],
    struct CbgNodeInfo node_info[]) {

    int i, j, k;
    unsigned long frequency_sum = 0;
    for (i = 0; i < 256; i ++) {
        node_info[i].frequency = frequency_table[i];
        node_info[i].valid = frequency_table[i] > 0;
        node_info[i].left_node = i;
        node_info[i].right_node = i;
        frequency_sum += frequency_table[i];
    }

    for (i = 256; i < 511; i ++) {
        node_info[i].frequency = 0;
        node_info[i].valid = 0;
        node_info[i].left_node = -1;
        node_info[i].right_node = -1;
    }

    for (i = 256; i < 511; i ++) {
        unsigned long frequency = 0;
        int children[2];
        for (j = 0; j < 2; j ++) {
            int min = 0xffffffff;
            children[j] = -1;
            for (k = 0; k < i; k ++) {
                if (node_info[k].valid && node_info[k].frequency < min) {
                    min = node_info[k].frequency;
                    children[j] = k;
                }
            }
            if (children[j] != -1) {
                node_info[children[j]].valid = 0;
                frequency += node_info[children[j]].frequency;
            }
        }
        node_info[i].valid = 1;
        node_info[i].frequency = frequency;
        node_info[i].left_node = children[0];
        node_info[i].right_node = children[1];
        if (frequency == frequency_sum)
            break;
    }
    return i;
}

static void cbg_decompress_huffman(
    unsigned char **input,
    int last_node,
    struct CbgNodeInfo node_info[],
    unsigned long huffman_size,
    unsigned char *huffman) {

    unsigned long root = last_node;
    unsigned char mask = 0x80;
    int i;

    for (i = 0; i < huffman_size; i ++) {
        int node = root;
        while (node >= 256) {
            node = **input & mask
                ? node_info[node].right_node
                : node_info[node].left_node;
            mask >>= 1;
            if (!mask) {
                (*input) ++;
                mask = 0x80;
            }
        }
        huffman[i] = node;
    }
}

static void cbg_decompress_rle(
    unsigned long huffman_size,
    unsigned char *huffman,
    unsigned char *output) {

    unsigned char *huffman_ptr = huffman;
    unsigned char *huffman_guardian = huffman + huffman_size;
    unsigned char zero_flag = 0;
    while (huffman_ptr < huffman_guardian) {
        unsigned long length = cbg_read_variable_data(&huffman_ptr, huffman_guardian);
        if (zero_flag) {
            memset(output, 0, length);
            output += length;
        } else {
            memcpy(output, huffman_ptr, length);
            huffman_ptr += length;
            output += length;
        }
        zero_flag ^= 1;
    }
}

static void cbg_transform_colors(
    unsigned char *input,
    unsigned short width,
    unsigned short height,
    unsigned short bpp) {

    unsigned short channels = bpp >> 3;
    int y, x, i;

    unsigned char *input_ptr = input;
    unsigned char *left = &input[- channels];
    unsigned char *above = &input[- width * channels];

    //ignore 0,0
    input += channels;
    above += channels;
    left += channels;

    //add left to first row
    for (x = 1; x < width; x ++) {
        for (i = 0; i < channels; i ++) {
            *input += input[-channels];
            input ++;
            above ++;
            left ++;
        }
    }

    //add left and top to all other pixels
    for (y = 1; y < height; y ++) {
        for (i = 0; i < channels; i ++) {
            *input += *above;
            input ++;
            above ++;
            left ++;
        }

        for (x = 1; x < width; x ++) {
            for (i = 0; i < channels; i ++) {
                *input += (*left  + *above) >> 1;
                input ++;
                above ++;
                left ++;
            }
        }
    }
}

VALUE decode_cbg_pixels(
    VALUE _self,
    VALUE _width,
    VALUE _height,
    VALUE _bpp,
    VALUE _input) {

    unsigned short width = FIX2INT(_width);
    unsigned short height = FIX2INT(_height);
    unsigned short bpp = FIX2INT(_bpp);

    size_t input_size = RSTRING_LEN(_input);
    unsigned char *input = RSTRING_PTR(_input);
    unsigned char *input_ptr = input;

    unsigned long huffman_size = read_ulong(&input_ptr);
    unsigned long key = read_ulong(&input_ptr);
    unsigned long frequency_table_data_size = read_ulong(&input_ptr);
    input_ptr += 4;

    unsigned long frequency_table[256];
    cbg_read_frequency_table(
        &input_ptr,
        frequency_table_data_size,
        key,
        frequency_table);

    struct CbgNodeInfo node_info[511];
    int last_node = cbg_read_node_info(frequency_table, node_info);

    unsigned char *huffman = allocate(huffman_size);
    cbg_decompress_huffman(&input_ptr, last_node, node_info, huffman_size, huffman);

    size_t output_size = width * height * (bpp >> 3);
    unsigned char *output = allocate(output_size);
    cbg_decompress_rle(huffman_size, huffman, output);
    free(huffman);

    cbg_transform_colors(output, width, height, bpp);

    VALUE ret = rb_str_new((char*) output, output_size);
    free(output);
    return ret;
}

void Init_cbg_pixel_decoder() {
    rb_define_global_function("decode_cbg_pixels", decode_cbg_pixels, 4);
}
