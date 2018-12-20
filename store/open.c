#include "buzz.h"

typedef struct Htable {
	uint64_t h;
	int      len;
	ulong    nr;
	uchar   *v;
	struct Htable *nxt;
} Htable;

static Htable **htab;
static ulong hash_badness;
static ulong hash_mismatch;
static ulong last_stored;
static State *last_state;

ulong
store_init(uchar *p, ulong unused)
{	ulong need = (1UL<<B_width) * sizeof(Htable *);

	if (p)
	{	htab = (Htable **) p;
	} else
	{	htab = (Htable **) e_malloc(need);
	}
	B_mode = Open;
	get_last_state = store_last_state;

	return need;
}

Results
store_state(uchar *v, int len)
{	uint64_t h = hash(v, len);
	int slot = h%(1UL<<B_width);
	Htable *n;

	for (n = htab[slot]; n; n = n->nxt)
	{	if (n->h == h)
		{	if (n->len == len
			&&  memcmp(n->v, v, len) == 0)
			{	if (B_verbose)
				{	printf("old state %lu (%d)\n", n->nr, slot);
				}
				last_stored = slot;
				last_state  = (State *) n->v;
				return Old;
			}
			hash_mismatch++;
		}
		hash_badness++;
	}
	n = (Htable *) e_malloc(sizeof(Htable));
	n->len = len;
	n->h = h;
	n->nr = B_nstates;
	n->nxt = htab[slot];
	htab[slot] = n;
	n->v = (uchar *) e_malloc(len * sizeof(uchar));
	memcpy((void *) n->v, (void *) v, len);
	last_stored = slot;
	last_state  = (State *) n->v;
	return New;
}

State *
store_last_state(void)
{
	return last_state;
}

#include "open_common.c"
