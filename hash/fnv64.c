#include "buzz.h"

// FNV64
// based on http://floodyberry.com/noncryptohashzoo/FNV64.html
// Eric Martin, CS118, 2015

uint64_t
hash_s(uchar *v, int len, uint64_t seed)
{	const uint64_t c = 1099511628211;
	uint64_t h = 0xcbf29ce484222325 + seed + len;

	for (; len & ~1; len -= 2, v += 2)
	{	h = (((h * c) ^ v[0]) * c) ^ v[1];
	}
	if (len & 1)
	{	h = (h * c) ^ v[0];
	}

	return h;
}

uint64_t
hash(uchar *v, int len)
{
	return hash_s(v, len, 0);
}
