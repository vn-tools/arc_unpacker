#ifndef FORMATS_ARC_PBGZ_ARCHIVE_CRYPT_H
#define FORMATS_ARC_PBGZ_ARCHIVE_CRYPT_H
#include "io.h"

typedef struct DecryptorContext
{
    uint8_t key;
    uint8_t step;
    size_t block_size;
    size_t limit;

    bool operator ==(const DecryptorContext &other) const;
} DecryptorContext;

void decrypt(
    IO &input, size_t size, IO &output, const DecryptorContext &context);

#endif
