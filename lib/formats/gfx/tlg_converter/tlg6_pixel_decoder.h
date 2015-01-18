#ifndef TLG6_PIXEL_DECODER_H
#define TLG6_PIXEL_DECODER_H
#include "ruby.h"

void tlg6_init_table();
VALUE decode_tlg6_pixels(VALUE _self, VALUE _header, VALUE _input);

#endif
