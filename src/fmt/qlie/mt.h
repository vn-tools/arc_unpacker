#ifndef AU_FMT_QLIE_MT_H
#define AU_FMT_QLIE_MT_H

namespace au {
namespace fmt {
namespace qlie {
namespace mt {

    void xor_state(const unsigned char *buff, unsigned long len);
    void init_genrand(unsigned long s);
    unsigned long genrand_int32(void);

} } } }

#endif
