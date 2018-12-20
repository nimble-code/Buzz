#include "buzz.h"

// 64-bit Jenkins hash, 1997
// http://burtleburtle.net/bob/c/lookup8.c

#define mix(a,b,c) \
	{ a -= b; a -= c; a ^= (c>>43); \
	  b -= c; b -= a; b ^= (a<<9);  \
	  c -= a; c -= b; c ^= (b>>8);  \
	  a -= b; a -= c; a ^= (c>>38); \
	  b -= c; b -= a; b ^= (a<<23); \
	  c -= a; c -= b; c ^= (b>>5);  \
	  a -= b; a -= c; a ^= (c>>35); \
	  b -= c; b -= a; b ^= (a<<49); \
	  c -= a; c -= b; c ^= (b>>11); \
	  a -= b; a -= c; a ^= (c>>12); \
	  b -= c; b -= a; b ^= (a<<18); \
	  c -= a; c -= b; c ^= (b>>22); \
	}

uint64_t
hash(uchar *v, const int len)
{	uint8_t  *bp;
	uint64_t  a, b, c, n;
	const uint64_t *k = (uint64_t *) v;

	n = len/WS;	// nr of words
	// extend to multiple of words, if needed
	a = WS - (len % WS);
	if (a > 0 && a < WS)
	{	n++;
		bp = v + len;
		switch (a) {
		case 7: *bp++ = 0; // fall thru
		case 6: *bp++ = 0; // fall thru
		case 5: *bp++ = 0; // fall thru
		case 4: *bp++ = 0; // fall thru
		case 3: *bp++ = 0; // fall thru
		case 2: *bp++ = 0; // fall thru
		case 1: *bp   = 0;
		case 0: break;
	}	}
	b = 0;
	c = 0x9e3779b97f4a7c13LL;
	while (n >= 3)
	{	a += k[0];
		b += k[1];
		c += k[2];
		mix(a,b,c);
		n -= 3;
		k += 3;
	}
	c += (((uint64_t) len)<<3);
	switch (n) {
	case 2: b += k[1];
	case 1: a += k[0];
	case 0: break;
	}
	mix(a,b,c);

	return c;
}

uint64_t
hash_s(uchar *v, int len, uint64_t seed)
{	uint8_t  *bp;
	uint64_t  a, b, c, n;
	const uint64_t *k = (uint64_t *) v;

	n = len/WS;
	a = WS - (len % WS);
	if (a > 0 && a < WS)
	{	n++;
		bp = v + len;
		switch (a) {
		case 7: *bp++ = 0; // fall thru
		case 6: *bp++ = 0; // fall thru
		case 5: *bp++ = 0; // fall thru
		case 4: *bp++ = 0; // fall thru
		case 3: *bp++ = 0; // fall thru
		case 2: *bp++ = 0; // fall thru
		case 1: *bp   = 0;
		case 0: break;
	}	}
	a = seed;
	b = 0;
	c = 0x9e3779b97f4a7c13LL;
	while (n >= 3)
	{	a += k[0];
		b += k[1];
		c += k[2];
		mix(a,b,c);
		n -= 3;
		k += 3;
	}
	c += (((uint64_t) len)<<3);
	switch (n) {
	case 2: b += k[1];
	case 1: a += k[0];
	case 0: break;
	}
	mix(a,b,c);

	return c;
}
