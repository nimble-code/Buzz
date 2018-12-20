#include "buzz.h"

// one-byte at a time, therefore slow
// http://stackoverflow.com/questions/13325125/lightweight-8-byte-hash-function-algorithm

uint64_t
hash(uchar *v, int len)
{	uint64_t h = 7;

	while (len-- > 0)
	{	h = (h << 5) - h + *v++;	// h*31 + *v++
	}
	return h;
}

uint64_t
hash_s(uchar *v, int len, uint64_t seed)
{	uint64_t h = seed;

	while (len-- > 0)
	{	h = (h << 5) - h + *v++;	// h*31 + *v++
	}
	return h;
}
