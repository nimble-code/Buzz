#include "buzz.h"

typedef struct Htable {
	uint64_t h;
	int      len;
	ulong    nr;
	uchar   *v;
} Htable;

static Htable *htab;
static ulong   hash_badness;
static ulong   hash_mismatch;
static ulong   last_stored;

ulong
store_init(uchar *p, ulong unused)
{	ulong need = (1UL<<B_width) * sizeof(Htable);

	if (p)
	{	htab = (Htable *) p;
	} else
	{	htab = (Htable *) e_malloc(need);
	}
	B_mode = Closed_notrace;
	get_last_state = store_last_state;

	return need;
}

Results
store_state(uchar *v, int len)
{	uint64_t h = hash(v, len);
	ulong slot = h%(1UL<<B_width);
	int try;
	static int warned = 0;

	for (try = 0; try < B_maxtry; try++)
	{	if (!htab[slot].len)
		{	htab[slot].len = len;
			htab[slot].v = (uchar *) e_malloc(len*sizeof(uchar));
			memcpy((void *) htab[slot].v, (void *) v, len);
			htab[slot].h = h;
			htab[slot].nr = B_nstates;
			if (B_verbose)
			{	printf("new state %lu (%lu)\n", htab[slot].nr, slot);
			}
			last_stored = slot;
			return New;
		}
		if (htab[slot].h == h)
		{	if (htab[slot].len == len
			&&  memcmp((void *) htab[slot].v, (void *) v, len) == 0)
			{	if (B_verbose)
				{	printf("old state %lu (%lu)\n", htab[slot].nr, slot);
				}
				last_stored = slot;
				return Old;
			}
			hash_mismatch++;
		}
	  	h = hash_s(v, len, h + try);
		slot = h%(1UL<<B_width);
		hash_badness++;
	}
	if (!warned)
	{	warned++;
		store_full();
	}
	return Unknown;
}

State *
store_last_state(void)
{
	return (State *) htab[last_stored].v;
}

#include "closed_common.c"
