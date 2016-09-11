#include "dec/shiina_rio/warc/plugin.h"

using namespace au;
using namespace au::dec::shiina_rio;
using namespace au::dec::shiina_rio::warc;

void BaseExtraCrypt::decrypt(bstr &data, const u32 flags) const
{
    if (data.size() < min_size())
        return;
    if ((flags & 0x202) == 0x202)
        pre_decrypt(data);
    if ((flags & 0x204) == 0x204)
        post_decrypt(data);
}
