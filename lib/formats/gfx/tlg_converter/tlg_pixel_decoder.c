#include "tlg5_pixel_decoder.h"
#include "tlg6_pixel_decoder.h"

void Init_tlg_pixel_decoder() {
    tlg6_init_table();
    rb_define_global_function("tlg5_decode_pixels", decode_tlg5_pixels, 2);
    rb_define_global_function("tlg6_decode_pixels", decode_tlg6_pixels, 2);
}
