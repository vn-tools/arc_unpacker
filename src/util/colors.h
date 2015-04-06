#ifndef UTIL_COLORS_H
#define UTIL_COLORS_H
#include <cstdint>

uint32_t rgb565(uint16_t word);
uint32_t rgba5551(uint16_t word);
uint32_t rgba4444(uint16_t word);
uint32_t rgba_gray(uint8_t byte);

void set_channel(uint32_t &color, uint8_t channel, uint8_t value);
uint8_t get_channel(uint32_t color, uint8_t channel);

void split_channels(uint32_t color, uint8_t channels[4]);
void merge_channels(uint8_t channels[4], uint32_t &color);

#endif
