#include "buzz.h"
#include "y.tab.h"

static ulong	 q_put = 0;
static ulong	 q_get = 0;
static long	 depth, mdepth;

static int put_queue(State *);

static void
bfs_stats(void)
{
	fprintf(stderr, "\t%ld maxdepth, %lu states (%luM), %lu revisits (%luM), ",
		mdepth,
		B_nstates, B_nstates/(1000*1000),
		B_ntruncs, B_ntruncs/(1000*1000));
	fprintf(stderr, "%lu MB mem used\n", mem_MB_used());
	store_stats();
}

void
print_state(const char *m, int with_trace)
{
	record_time();
	fprintf(stderr, "%s\n", m);
	fprintf(stderr, "\t%s search, %s storage\n",
		search_mode(), store_mode());
	bfs_stats();
	fprintf(stderr, "\t#errors %d, #procs %d, ", B_ecount, B_nproc);
	trans_stats();
	store_test();

	if (with_trace)
	{	get_stack((ulong)(qidx(q_get,toggle)-1), depth);
	}

	unvisited(B_ast);
	report_time(B_unreached);
	exit(B_ecount);
}

static int
has_successors(State *s, short timeout_enabled)
{	short n, t, stopped = 0, executed = 0, po_safe;

	for (n = 0; n < B_nproc; n++)	// all processes
	{	B_procname = B_pnames[n]->nm;
		if (!B_state.s[n])
		{	stopped++;
			continue;
		}
		po_safe = (B_state.s[n]->alt[0]->tag & PO_Safe);
		for (t = 0; t < Fmax && B_state.s[n]->alt[t]; t++) // all transitions
		{	AST *a = B_state.s[n]->alt[t];
			B_pid = n;
			a->tag |= Reached;
			if ((timeout_enabled && a->tok == TIMEOUT)
			||  step(a, executed))
			{	executed++;
				B_state.s[n] = a->nxt;
				if (put_queue(&B_state))
				{	set_parent(qidx(q_get,toggle), n, t);
				}
				memcpy((void *) &B_state, (void *) s, B_ssize);
		}	}
		if (executed && po_safe)
		{	break;
	}	}

	if (B_verbose)
	{	fprintf(stderr, "executed %d stopped %d nproc %d, timeout %d\n",
			executed, stopped, B_nproc, timeout_enabled);
	}
		
	return (executed || stopped == B_nproc);
}

static void
explore(State *s)
{
	assert(s);
	memcpy((void *) &B_state, (void *) s, B_ssize);

	if (!has_successors(s, NO_TIMEOUT)
	&&  !has_successors(s, USE_TIMEOUT)
	&&  ++B_ecount == B_ewant)
	{	print_state("deadlock state", 1);
	}
}
