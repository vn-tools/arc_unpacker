/*
   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

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

#include "mt.h"

const int N = 624;
const int M = 397;
const unsigned long MATRIX_A = 0x9908b0dfUL;
const unsigned long UPPER_MASK = 0x80000000UL;
const unsigned long LOWER_MASK = 0x7fffffffUL;

static unsigned long mts[N];
static int mti = N + 1;

void au::util::mt::init_genrand(unsigned long s)
{
    mts[0] = s & 0xffffffffUL;
    for (mti = 1; mti < N; mti++)
    {
        mts[mti] = (1812433253UL * (mts[mti - 1] ^ (mts[mti - 1] >> 30)) + mti);
        mts[mti] &= 0xffffffffUL;
    }
}

void au::util::mt::init_by_array(unsigned long init_key[], int key_length)
{
    init_genrand(19650218UL);
    int i = 1;
    int j = 0;
    int k = (N > key_length ? N : key_length);
    for (; k; k--)
    {
        mts[i] = (mts[i] ^ ((mts[i - 1] ^ (mts[i - 1] >> 30)) * 1664525UL))
            + init_key[j] + j;
        mts[i] &= 0xffffffffUL;
        i++;
        j++;
        if (i >= N)
        {
            mts[0] = mts[N - 1];
            i = 1;
        }
        if (j >= key_length)
        {
            j = 0;
        }
    }
    for (k = N - 1; k; k--)
    {
        mts[i] = (mts[i] ^ ((mts[i - 1] ^ (mts[i - 1] >> 30)) * 1566083941UL))
            - i;
        mts[i] &= 0xffffffffUL;
        i++;
        if (i >= N)
        {
            mts[0] = mts[N - 1];
            i = 1;
        }
    }

    mts[0] = 0x80000000UL;
}

unsigned long au::util::mt::genrand_int32()
{
    unsigned long y;
    static unsigned long mag01[2] = { 0x0UL, MATRIX_A };

    if (mti >= N)
    {
        int kk;

        if (mti == N + 1)
            init_genrand(5489UL);

        for (kk = 0; kk < N - M; kk++)
        {
            y = (mts[kk] & UPPER_MASK) | (mts[kk + 1] & LOWER_MASK);
            mts[kk] = mts[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (; kk < N - 1; kk++)
        {
            y = (mts[kk] & UPPER_MASK) | (mts[kk + 1] & LOWER_MASK);
            mts[kk] = mts[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mts[N - 1] & UPPER_MASK) | (mts[0] & LOWER_MASK);
        mts[N - 1] = mts[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];
        mti = 0;
    }

    y = mts[mti++];

    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

long au::util::mt::genrand_int31()
{
    return (long)(genrand_int32() >> 1);
}

double au::util::mt::genrand_real1()
{
    return genrand_int32() * (1.0 / 4294967295.0);
}

double au::util::mt::genrand_real2()
{
    return genrand_int32() * (1.0 / 4294967296.0);
}

double au::util::mt::genrand_real3()
{
    return (((double)genrand_int32()) + 0.5) * (1.0 / 4294967296.0);
}

double au::util::mt::genrand_res53()
{
    unsigned long a = genrand_int32() >> 5;
    unsigned long b = genrand_int32() >> 6;
    return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0);
}
