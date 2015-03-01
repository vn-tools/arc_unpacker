#ifndef FORMATS_NITROPLUS_NPA_FILTERS_FILTER_H
#define FORMATS_NITROPLUS_NPA_FILTERS_FILTER_H
#include <cstdint>

namespace Formats
{
    namespace Nitroplus
    {
        namespace NpaFilters
        {
            typedef struct
            {
                const unsigned char *permutation;
                uint32_t data_key;
                uint32_t (*file_name_key)(uint32_t key1, uint32_t key2);
            } Filter;
        }
    }
}

#endif
