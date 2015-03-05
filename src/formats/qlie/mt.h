#ifndef FORMATS_QLIE_MT_H
#define FORMATS_QLIE_MT_H

namespace Formats
{
    namespace QLiE
    {
        void mt_xor_state(const unsigned char *buff, unsigned long len);
        void mt_init_genrand(unsigned long s);
        unsigned long mt_genrand_int32(void);
    }
}

#endif
