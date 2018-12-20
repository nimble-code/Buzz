#include "buzz.h"

// FNV-1a
// based on http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-1a
// Dhiraj Holden, CS118, 2015

uint64_t
hash(uchar* v, const int len)
{	uint64_t h = 0xcbf29ce484222325;

	for( int i = 0; i < len; i++ )
	{	h ^= v[i];
		h *= 0x100000001b3;
	}
	return h;
}


uint64_t
hash_s(uchar* v, int len, uint64_t seed)
{	uint64_t h = 0xcbf29ce484222325;

	for( int i = 0; i < 7; i++ )
	{	h ^= (seed & 0xFF);
		h *= 0x100000001b3;
	}
	for( int i = 0; i < len; i++ )
	{	h ^= v[i];
		h *= 0x100000001b3;
	}
	return h;
}
