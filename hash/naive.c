#include "buzz.h"

uint64_t
hash(uchar *v, int len)
{	uint64_t h = 0;

	while (len-- > 0)
	{	h += *v++;
	}
	return h;
}

uint64_t
hash_s(uchar *v, int len, uint64_t seed)
{	uint64_t h = seed;

	while (len-- > 0)
	{	h += *v++;
	}
	return h;
}
