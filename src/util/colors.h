#ifndef UTIL_COLORS_H
#define UTIL_COLORS_H
#include <cstdint>

uint32_t rgb565(uint16_t word);
uint32_t rgba5551(uint16_t word);
uint32_t rgba4444(uint16_t word);
uint32_t rgba_gray(uint8_t byte);

#endif
