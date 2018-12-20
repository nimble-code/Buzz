#include "buzz.h"

// http://www.cse.yorku.ca/~oz/hash.html

uint64_t
hash(uchar *v, int len)
{	uint64_t h = 5381;

	while (len-- > 0)
	{	h = ((h << 5) + h) + *v++; // h * 33 + c
	}

	return h;
}

uint64_t
hash_s(uchar *v, int len, uint64_t seed)
{	uint64_t h = seed;

	while (len-- > 0)
	{	h = ((h << 5) + h) + *v++; // h * 33 + c
	}

	return h;
}
