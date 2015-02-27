#ifndef FORMATS_ARC_PBGZ_ARCHIVE_CRYPT_H
#define FORMATS_ARC_PBGZ_ARCHIVE_CRYPT_H
#include "io.h"

typedef struct
{
    uint8_t key;
    uint8_t step;
    size_t block_size;
    size_t limit;
} DecryptorContext;

void decrypt(
    IO &input, size_t size, IO &output, const DecryptorContext &context);

#endif
