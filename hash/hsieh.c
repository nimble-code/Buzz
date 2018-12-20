#include "buzz.h"

// 32-bit hash, based on Paul Hsieh's function
// http://www.azillionmonkeys.com/qed/hash.html

#define get16bits(d)	(*((const uint16_t *) (d)))

static uint32_t
hash32(uchar *v, uint32_t n, uint32_t s)
{	uint32_t t;
	uint32_t h = s;

	for ( ; n > 0; n--, v += 2*sizeof(uint16_t))
	{	h += get16bits(v);
		t  = (get16bits(v+sizeof(uint16_t)) << 11) ^ h;
         	h  = (h << 16) ^ t;
		h +=  h >> 11;
	}
	h ^= h << 3; h += h >> 5;
	h ^= h << 4; h += h >>17;
	h ^= h <<25; h += h >> 6;

	return h;
}

uint64_t
hash_s(uchar *v, const int nbytes, uint64_t s)
{	uchar  *p;
	uint32_t n;
	uint64_t a;

	assert(WS == 8);	// static assertion
	n = nbytes/WS;		// nr of 64-bit words

	a = WS - (nbytes % WS);	// extend to multiple of words
	if (a > 0 && a < WS)
	{	n++;
		p = v + nbytes;
		switch (a) {
		case 7: *p++ = 0; // fall thru
		case 6: *p++ = 0; // fall thru
		case 5: *p++ = 0; // fall thru
		case 4: *p++ = 0; // fall thru
		case 3: *p++ = 0; // fall thru
		case 2: *p++ = 0; // fall thru
		case 1: *p   = 0;
		case 0: break;
	}	}

	return (uint64_t) hash32(v, n, (uint32_t) s);
}

uint64_t
hash(uchar *v, const int nbytes)
{
	return hash_s(v, nbytes, (uint64_t) nbytes);
}
