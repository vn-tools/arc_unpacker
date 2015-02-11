#ifndef FORMATS_ARC_NPA_ARCHIVE_NPA_FILTER
#define FORMATS_ARC_NPA_ARCHIVE_NPA_FILTER
#include <cstdint>

typedef struct
{
    const unsigned char *permutation;
    uint32_t data_key;
    uint32_t (*file_name_key)(uint32_t key1, uint32_t key2);
} NpaFilter;

#endif
