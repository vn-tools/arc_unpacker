#ifndef FORMATS_NITROPLUS_NPA_FILTERS_FILTER_H
#define FORMATS_NITROPLUS_NPA_FILTERS_FILTER_H
#include "types.h"

namespace Formats
{
    namespace Nitroplus
    {
        namespace NpaFilters
        {
            typedef struct
            {
                const u8 *permutation;
                u32 data_key;
                u32 (*file_name_key)(u32 key1, u32 key2);
            } Filter;
        }
    }
}

#endif
