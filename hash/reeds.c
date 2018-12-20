#include "buzz.h"

// based on the 32-bit hash function
// from the first version of Spin

uint64_t
hash_s(uchar *v, const int len, uint64_t s)
{	uint64_t  z = s;
	int64_t *q = (int64_t *) v; // assumes alignment
	int64_t  h = (len+(WS-1))/WS;
	int64_t  m = -1;
	int64_t  n = -1;

	do {	m += m;
		if (m < 0)
		{	m ^= z;
		}
		m ^= *q++;
	} while (--h > 0);

	// for additional 32-bits
	// same in reverse order

	q = (int64_t *) (v + len);
	h = (len+(WS-1))/WS;

	do {	n += n;
		if (n < 0)
		{	n ^= z;
		}
		n ^= *--q;
	} while (--h > 0);


	return (uint64_t) (m<<32) ^ n;
}

uint64_t
hash(uchar *v, const int len)
{
	return hash_s(v, len, 0x88888EEFL);
}
