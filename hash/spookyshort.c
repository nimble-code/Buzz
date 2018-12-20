#include "buzz.h"

// short SpookyHash, rewritten in C. See:
// based on:
//	http://burtleburtle.net/bob/c/SpookyV2.h
//	http://burtleburtle.net/bob/c/SpookyV2.cpp
// assumes that v is aligned and/or that unaligned reads are not too costly
//
// random numbers from random.org
// sc_const should be odd and not too nonrandom

// Yichi Zhang, CS118, 2015


#define sc_const 0xd04706fd1d986e21LL

#define Rot64(x,k) (x<<k)|(x>>(64-k));

#define ShortMix(h0,h1,h2,h3) {              \
    h2 = Rot64(h2,50);  h2 += h3;  h0 ^= h2; \
    h3 = Rot64(h3,52);  h3 += h0;  h1 ^= h3; \
    h0 = Rot64(h0,30);  h0 += h1;  h2 ^= h0; \
    h1 = Rot64(h1,41);  h1 += h2;  h3 ^= h1; \
    h2 = Rot64(h2,54);  h2 += h3;  h0 ^= h2; \
    h3 = Rot64(h3,48);  h3 += h0;  h1 ^= h3; \
    h0 = Rot64(h0,38);  h0 += h1;  h2 ^= h0; \
    h1 = Rot64(h1,37);  h1 += h2;  h3 ^= h1; \
    h2 = Rot64(h2,62);  h2 += h3;  h0 ^= h2; \
    h3 = Rot64(h3,34);  h3 += h0;  h1 ^= h3; \
    h0 = Rot64(h0,5);   h0 += h1;  h2 ^= h0; \
    h1 = Rot64(h1,36);  h1 += h2;  h3 ^= h1; \
  }

#define ShortEnd(h0,h1,h2,h3) {              \
    h3 ^= h2;  h2 = Rot64(h2,15);  h3 += h2; \
    h0 ^= h3;  h3 = Rot64(h3,52);  h0 += h3; \
    h1 ^= h0;  h0 = Rot64(h0,26);  h1 += h0; \
    h2 ^= h1;  h1 = Rot64(h1,51);  h2 += h1; \
    h3 ^= h2;  h2 = Rot64(h2,28);  h3 += h2; \
    h0 ^= h3;  h3 = Rot64(h3,9);   h0 += h3; \
    h1 ^= h0;  h0 = Rot64(h0,47);  h1 += h0; \
    h2 ^= h1;  h1 = Rot64(h1,54);  h2 += h1; \
    h3 ^= h2;  h2 = Rot64(h2,32);  h3 += h2; \
    h0 ^= h3;  h3 = Rot64(h3,25);  h0 += h3; \
    h1 ^= h0;  h0 = Rot64(h0,63);  h1 += h0; \
  }

uint64_t
hash(uchar *v, int len)
{
	return hash_s(v, len, 0x46d2e99ef84100efLL);
}

uint64_t
hash_s(uchar *v, int len, uint64_t seed)
{
  uint64_t a,b,c,d;
  size_t remainder = len&31;
  union {
    uchar *p8;
    uint32_t *p32;
    uint64_t *p64;
  } u;
  u.p8 = v;
  a = seed;
  b = 0xa130191afc91acf3LL;
  c = sc_const;
  d = sc_const;

  if (len > 15) {
    const uint64_t *end = u.p64 + (len/32)*4;
    
    // handle all complete sets of 32 bytes
    for (; u.p64 < end; u.p64 += 4) {
      c += u.p64[0];
      d += u.p64[1];
      ShortMix(a,b,c,d);
      a += u.p64[2];
      b += u.p64[3];
    }
    
    //Handle the case of 16+ remaining bytes.
    if (remainder >= 16) {
      c += u.p64[0];
      d += u.p64[1];
      ShortMix(a,b,c,d);
      u.p64 += 2;
      remainder -= 16;
    }
  }
  d += ((uint64_t)len) << 56;
  switch (remainder)
    {
    case 15:
    d += ((uint64_t)u.p8[14]) << 48;
    case 14:
        d += ((uint64_t)u.p8[13]) << 40;
    case 13:
        d += ((uint64_t)u.p8[12]) << 32;
    case 12:
        d += u.p32[2];
        c += u.p64[0];
        break;
    case 11:
        d += ((uint64_t)u.p8[10]) << 16;
    case 10:
        d += ((uint64_t)u.p8[9]) << 8;
    case 9:
        d += (uint64_t)u.p8[8];
    case 8:
        c += u.p64[0];
        break;
    case 7:
        c += ((uint64_t)u.p8[6]) << 48;
    case 6:
        c += ((uint64_t)u.p8[5]) << 40;
    case 5:
        c += ((uint64_t)u.p8[4]) << 32;
    case 4:
        c += u.p32[0];
        break;
    case 3:
        c += ((uint64_t)u.p8[2]) << 16;
    case 2:
        c += ((uint64_t)u.p8[1]) << 8;
    case 1:
        c += (uint64_t)u.p8[0];
        break;
    case 0:
        c += sc_const;
        d += sc_const;
    }
  return a;
}

#undef ShortMix
#undef ShortEnd
#undef sc_const
