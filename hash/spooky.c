#include "buzz.h"

// SpookyHash, minus the short option, rewritten in C. See:
// based on:
//	http://burtleburtle.net/bob/c/SpookyV2.h
//	http://burtleburtle.net/bob/c/SpookyV2.cpp
// assumes that v is aligned and/or that unaligned reads are not too costly
//
// should be faster on longer keys (threshold 192 bytes)
//
// random numbers from random.org
// sc_const should be odd and not too nonrandom
// k1 and k2 are arbitrary

// Yichi Zhang, CS118, 2015

#define sc_const 0xd04706fd1d986e21LL
#define k1       0x46d2e99ef84100efLL
#define k2       0xa130191afc91acf3LL

#define Rot64(x,k) ((x<<k)|(x>>(64-k)))

#define Mix(data,s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11) { \
    s0 += data[0];   s2 ^= s10; s11 ^= s0;  s0 = Rot64(s0,11);   s11 += s1; \
    s1 += data[1];   s3 ^= s11; s0 ^= s1;   s1 = Rot64(s1,32);   s0 += s2;  \
    s2 += data[2];   s4 ^= s0;  s1 ^= s2;   s2 = Rot64(s2,43);   s1 += s3;  \
    s3 += data[3];   s5 ^= s1;  s2 ^= s3;   s3 = Rot64(s3,31);   s2 += s4;  \
    s4 += data[4];   s6 ^= s2;  s3 ^= s4;   s4 = Rot64(s4,17);   s3 += s5;  \
    s5 += data[5];   s7 ^= s3;  s4 ^= s5;   s5 = Rot64(s5,28);   s4 += s6;  \
    s6 += data[6];   s8 ^= s4;  s5 ^= s6;   s6 = Rot64(s6,39);   s5 += s7;  \
    s7 += data[7];   s9 ^= s5;  s6 ^= s7;   s7 = Rot64(s7,57);   s6 += s8;  \
    s8 += data[8];   s10 ^= s6; s7 ^= s8;   s8 = Rot64(s8,55);   s7 += s9;  \
    s9 += data[9];   s11 ^= s7; s8 ^= s9;   s9 = Rot64(s9,54);   s8 += s10; \
    s10 += data[10]; s0 ^= s8;  s9 ^= s10;  s10 = Rot64(s10,22); s9 += s11; \
    s11 += data[11]; s1 ^= s9;  s10 ^= s11; s11 = Rot64(s11,46); s10 += s0; \
  }

#define EndPartial(h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11) { \
    h11+= h1;  h2 ^= h11; h1 = Rot64(h1,44);  \
    h0 += h2;  h3 ^= h0;  h2 = Rot64(h2,15);  \
    h1 += h3;  h4 ^= h1;  h3 = Rot64(h3,34);  \
    h2 += h4;  h5 ^= h2;  h4 = Rot64(h4,21);  \
    h3 += h5;  h6 ^= h3;  h5 = Rot64(h5,38);  \
    h4 += h6;  h7 ^= h4;  h6 = Rot64(h6,33);  \
    h5 += h7;  h8 ^= h5;  h7 = Rot64(h7,10);  \
    h6 += h8;  h9 ^= h6;  h8 = Rot64(h8,13);  \
    h7 += h9;  h10^= h7;  h9 = Rot64(h9,38);  \
    h8 += h10; h11^= h8;  h10= Rot64(h10,53); \
    h9 += h11; h0 ^= h9;  h11= Rot64(h11,42); \
    h10+= h0;  h1 ^= h10; h0 = Rot64(h0,54);  \
  }

#define End(data,h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11) { \
    h0 += data[0]; h1 += data[1]; h2 += data[2];   h3 += data[3];   \
    h4 += data[4]; h5 += data[5]; h6 += data[6];   h7 += data[7];   \
    h8 += data[8]; h9 += data[9]; h10 += data[10]; h11 += data[11]; \
    EndPartial(h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);              \
    EndPartial(h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);              \
    EndPartial(h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);              \
  }

uint64_t
hash(uchar *v, int len)
{
	return hash_s(v, len, k1);
}

uint64_t
hash_s(uchar *v, int len, uint64_t seed)
{
	uint64_t h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11;
	uint64_t buf[12];
	uint64_t *end;
	size_t remainder;
	union {
		uchar *p8;
		uint64_t *p64;
		size_t i;
	} u;

	h0 = h3 = h6 = h9 = seed;
	h1 = h4 = h7 = h10 = k2;
	h2 = h5 = h8 = h11 = sc_const;
  
	u.p8 = v;
	end = u.p64 + (len/(8*12))*12;

	while (u.p64 < end) {
		Mix(u.p64,h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
		u.p64 += 12;
	}

	remainder = (len - ((uchar *)end-v));
	memcpy(buf, end, remainder);
	memset(((uchar *)buf)+remainder, 0, 8*12-remainder);
	((uchar *)buf)[8*12-1] = remainder;
  
	End(buf,h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);

	return h0^h1;
}

#undef sc_const
#undef k1
#undef k2
