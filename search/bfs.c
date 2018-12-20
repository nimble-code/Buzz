#include "buzz.h"

static State	*queue;
static ulong    *q_slot;
static int	 toggle;

#define qref(a,t)	(queue[(t) *B_maxgen + (a)])
#define qidx(a,t)	(q_slot[(t)*B_maxgen + (a)])

#include "bfs_common.c"

static void
queue_init(void)
{
	queue  = (State *) e_malloc((size_t) (B_maxgen * 2L * B_ssize));
	q_slot = (ulong *) e_malloc((size_t) (B_maxgen * 2L * sizeof(ulong)));
}

static int
put_queue(State *s)
{
	assert(s);
	if (store_state((uchar *) s, B_ssize) != New)
	{	B_ntruncs++;
		return 0;
	}
	B_nstates++;
	if (q_put < B_maxgen)
	{	if (B_verbose)
		{	fprintf(stderr, "Put %lu\t", q_put);
			fprintf(stderr, "slot %lu\n", store_last_slot());
		}
		memcpy((void *) &(qref(q_put,1-toggle)), (void *) s, B_ssize);
		qidx(q_put, 1-toggle) = store_last_slot()+1;
		q_put++;
	} else
	{	static int warned = 0;
		if (!warned)
		{	warned = 1;
			fprintf(stderr, "B_maxgen (%lu) too small, losing states\n",
				B_maxgen);
	}	}
	return 1;
}

static State *
get_queue(void)
{	State *s = 0;

	if (q_get < B_maxgen && qidx(q_get,toggle))
	{	s = &(qref(q_get,toggle));
		if (B_verbose)
		{	fprintf(stderr, "Get %lu\t", q_get);
			fprintf(stderr, "slot %lu\n", qidx(q_get, toggle)-1);
	}	}

	return s;	// 0 if no state there
}

static void
clr_queue(void)
{
	if (B_verbose>1)
	{	fprintf(stderr, "Clr %lu\n", q_get);
	}
	assert(qidx(q_get,toggle));
	qidx(q_get,toggle) = 0;
	q_get++;
}

static void
show_state(long d, State *s, ushort n, ushort t)
{	AST *a;

	if (!s)
	{	return;
	}

	assert(n >= 0 && n < B_nproc);
	assert(t >= 0 && t < Fmax);

	a = s->s[n];
	assert(a != NULL);

	if (!B_verbose && !a->attr)
	{	return;
	}
	B_procname = B_pnames[n]->nm;
	fprintf(stderr, "%3ld\t%s:%03d:\tproc %s :: ",
		d, a->fnm, a->ln, B_procname);
	if (a->alt[t] && a->alt[t]->attr)	// dst
	{	trans_print(a->alt[t]);
	} else
	{	fprintf(stderr, "skip (%d)\n", a->alt[t]?a->alt[t]->tok:-1);
	}
}

static void
bfs_stacktrace(ulong slot, long d)
{	State *s;
	long   i;
	ushort n, t;

	fprintf(stderr, "\nTrace (%ld steps, in reverse order):\n",
		depth);

	set_last_slot(slot);
	for (i = d; i > 0 && slot; i--)
	{	n = get_last_pid();
		t = get_last_trans();
		slot = get_parent(slot)-1;
		set_last_slot(slot);
		s = get_last_state();
		show_state(i, s, n, t);
	}
	fprintf(stderr, "\nfinal state:\n");
	trans_state();
}

void
exec(void)
{	State *s;

	B_search       = Bfs;
	get_stack      = bfs_stacktrace;
	queue_init();
	store_init(0, 0);

	if (B_mode != Closed)
	{	fatal("search=bfs requires store=closed");
	}

	(void) put_queue(&B_state);
	do {
		q_get = q_put = 0;
		toggle = 1 - toggle;

		while ((s = get_queue()) != NULL)
		{	explore(s);
			clr_queue();
		}

		if (++depth > mdepth)
		{	mdepth = depth;
		}
		if (B_verbose || depth%10 == 0)
		{	fprintf(stderr, "depth %3ld, nstates %6lu (%luM), new %6lu (%luM)\n",
				depth,
				B_nstates, B_nstates/(1000*1000),
				q_put, q_put/(1000*1000));
		}
		if (depth >= B_maxdepth)
		{	break;
		}
	} while (q_put > 0);
	print_state("search completed", 0);
}
