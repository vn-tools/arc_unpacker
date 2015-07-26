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

#include "fmt/qlie/mt.h"

const int N = 64;
const int M = 39;
const unsigned long MATRIX_A = 0x9908B0DFul;
const unsigned long UPPER_MASK = 0x80000000ul;
const unsigned long LOWER_MASK = 0x7FFFFFFFul;

static unsigned long mts[N];
static int mti = N + 1;

void au::fmt::qlie::mt::xor_state(const unsigned char* buff, unsigned long len)
{
    unsigned long *words = (unsigned long*) buff;
    unsigned long word_count = len / 4;
    unsigned long i;

    if (word_count > N)
        word_count = N;

    for (i = 0; i < word_count; i++)
        mts[i] ^= words[i];
}

void au::fmt::qlie::mt::init_genrand(unsigned long s)
{
    mts[0] = s & 0xFFFFFFFFul;
    for (mti = 1; mti < N; mti++)
    {
        mts[mti] = (1712438297ul * (mts[mti - 1] ^ (mts[mti - 1] >> 30)) + mti);
        mts[mti] &= 0xFFFFFFFFul;
    }
}

unsigned long au::fmt::qlie::mt::genrand_int32()
{
    unsigned long y;
    static unsigned long mag01[2] = { 0x0ul, MATRIX_A };

    if (mti >= N)
    {
        int kk;

        if (mti == N + 1)
            au::fmt::qlie::mt::init_genrand(5489ul);

        for (kk = 0; kk < N - M; kk++)
        {
            y = (mts[kk] & UPPER_MASK) | ((mts[kk + 1] & LOWER_MASK) >> 1);
            mts[kk] = mts[kk + M] ^ y ^ mag01[mts[kk + 1] & 0x1ul];
        }
        for (; kk < N - 1; kk++)
        {
            y = (mts[kk] & UPPER_MASK) | ((mts[kk + 1] & LOWER_MASK) >> 1);
            mts[kk] = mts[kk + (M - N)] ^ y ^ mag01[mts[kk + 1] & 0x1ul];
        }
        y = (mts[N - 1] & UPPER_MASK) | ((mts[0] & LOWER_MASK) >> 1);
        mts[N - 1] = mts[M - 1] ^ y ^ mag01[mts[N - 1] & 0x1ul];
        mti = 0;
    }

    y = mts[mti++];

    y ^= (y >> 11);
    y ^= (y << 7) & 0x9C4F88E3ul;
    y ^= (y << 15) & 0xE7F70000ul;
    y ^= (y >> 18);

    return y;
}
