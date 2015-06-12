/*
   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   2009/09/25 - Modifications by asmodean to match the non-standard
                implementation found in FilePackVer3.0 archives used
                by Signal Hearts.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "formats/qlie/mt.h"

const int N = 64;
const int M = 39;
const unsigned long MATRIX_A = 0x9908b0dfUL;
const unsigned long UPPER_MASK = 0x80000000UL;
const unsigned long LOWER_MASK = 0x7fffffffUL;

static unsigned long mt[N];
static int mti = N + 1;

void Formats::QLiE::mt_xor_state(const unsigned char* buff, unsigned long len)
{
    unsigned long *words = (unsigned long*) buff;
    unsigned long word_count = len / 4;
    unsigned long i;

    if (word_count > N)
        word_count = N;

    for (i = 0; i < word_count; i++)
        mt[i] ^= words[i];
}

void Formats::QLiE::mt_init_genrand(unsigned long s)
{
    mt[0] = s & 0xffffffffUL;
    for (mti = 1; mti < N; mti++)
    {
        mt[mti] = (1712438297UL * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
        mt[mti] &= 0xffffffffUL;
    }
}

unsigned long Formats::QLiE::mt_genrand_int32()
{
    unsigned long y;
    static unsigned long mag01[2] = { 0x0UL, MATRIX_A };

    if (mti >= N)
    {
        int kk;

        if (mti == N + 1)
            mt_init_genrand(5489UL);

        for (kk = 0; kk < N - M; kk++)
        {
            y = (mt[kk] & UPPER_MASK) | ((mt[kk + 1] & LOWER_MASK) >> 1);
            mt[kk] = mt[kk + M] ^ y ^ mag01[mt[kk + 1] & 0x1UL];
        }
        for (; kk < N - 1; kk++)
        {
            y = (mt[kk] & UPPER_MASK) | ((mt[kk + 1] & LOWER_MASK) >> 1);
            mt[kk] = mt[kk + (M - N)] ^ y ^ mag01[mt[kk + 1] & 0x1UL];
        }
        y = (mt[N - 1] & UPPER_MASK) | ((mt[0] & LOWER_MASK) >> 1);
        mt[N - 1] = mt[M - 1] ^ y ^ mag01[mt[N - 1] & 0x1UL];
        mti = 0;
    }

    y = mt[mti++];

    y ^= (y >> 11);
    y ^= (y << 7) & 0x9c4f88e3ul;
    y ^= (y << 15) & 0xe7f70000ul;
    y ^= (y >> 18);

    return y;
}
