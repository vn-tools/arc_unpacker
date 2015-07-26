#ifndef AU_FMT_NITROPLUS_NPA_FILTER_H
#define AU_FMT_NITROPLUS_NPA_FILTER_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace nitroplus {

    typedef struct
    {
        const u8 *permutation;
        u32 data_key;
        u32 (*file_name_key)(u32 key1, u32 key2);
    } NpaFilter;

} } }

#endif
