#include "buzz.h"

// rotational hash, similar to bernstein.c
// Sumanth Dathathri, CS118, 2015

uint64_t
hash(uchar *v, int len)
{	uint64_t h = 5381;
	int i;
	for ( i = 0; i < len; i++ )
	{	h^= ((h << 5) ^ (h >> 2))+ *v; // h * 33 + c
		v=v+1;
	}

	return h;
}

uint64_t
hash_s(uchar *v, int len, uint64_t seed)
{	uint64_t h = seed;
	int i;
	for ( i = 0; i < len; i++ )
	{	h^= ((h << 4) ^ (h >> 28))+ *v;
		v=v+1;
	}

	return h;
}
