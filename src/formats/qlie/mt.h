#ifndef FORMATS_QLIE_MT_H
#define FORMATS_QLIE_MT_H

void mt_xor_state(const unsigned char *buff, unsigned long len);
void init_genrand(unsigned long s);
unsigned long genrand_int32(void);

#endif
