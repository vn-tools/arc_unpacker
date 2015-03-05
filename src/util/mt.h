#ifndef UTIL_MT_H
#define UTIL_MT_H

void mt_init_genrand(unsigned long s);
void mt_init_by_array(unsigned long init_key[], int key_length);
unsigned long mt_genrand_int32();
long mt_genrand_int31();
double mt_genrand_real1();
double mt_genrand_real2();
double mt_genrand_real3();
double mt_genrand_res53();

#endif
