#include "buzz.h"

typedef struct Stack {
	State	*b;	// ptr to hash-table for backtracking
	short	 n;	// process selected
	short	 t;	// transition selected
} Stack;

static long	 depth = -1;
static long	 mdepth;
static int	 warned;
static Stack	*stack;

static void	dfs(void);

static void
dfs_stats(int final)
{
	if (final)
	{	fprintf(stderr, "\t");
	}
	fprintf(stderr, "%ld maxdepth, %lu states (%luM), %lu revisits (%luM), ",
		mdepth,
		B_nstates, B_nstates/(1000*1000),
		B_ntruncs, B_ntruncs/(1000*1000));
	fprintf(stderr, "%lu MB mem used\n", mem_MB_used());
	if (final)
	{	store_stats();
	}
}

static void
dfs_stacktrace(ulong unused, long d)
{	long i;
	int  t;

	if (d > 0)
	{	fprintf(stderr, "Stack (%ld steps):\n", d);
	}
	for (i = 0; i < d; i++)
	{	int n = stack[i].n;
		AST *a;

		assert(n < MaxProc && n >= 0);

		a = stack[i].b->s[n];
		assert(a != NULL);
		if (!B_verbose && !a->attr)	// dst
		{	continue;
		}
		fprintf(stderr, "%3ld\t%s:%02d:\tproc %s\t:: ",
			i, a->fnm, a->ln, B_pnames[n]->nm);
		t = stack[i].t;
		if (a->alt[t]->attr)		// dst
		{	trans_print(a->alt[t]);
		} else
		{	fprintf(stderr, "skip\n");
	}	}
}

void
print_state(const char *m, int with_trace)
{	int i;

	record_time();
	fprintf(stderr, "%s\n", m);
	fprintf(stderr, "\t%s search, %s storage\n",
		search_mode(), store_mode());
	dfs_stats(1);
	fprintf(stderr, "\t#errors %d, #procs %d, ", B_ecount, B_nproc);
	trans_stats();
	store_test();

	if (with_trace)
	{	fprintf(stderr, "final state:\n");
		for (i = 0; i < B_nproc; i++)
		{	if (B_state.s[i])
			{	fprintf(stderr, "\tproc %s: %s:%d:\n",
					B_pnames[i]->nm,
					B_state.s[i]->fnm,
					B_state.s[i]->ln);
			} else
			{	fprintf(stderr, "\tproc %s: -stopped-\n",
					B_pnames[i]->nm);
		}	}
		trans_state();
		get_stack(0, depth);
	}
	unvisited(B_ast);
	report_time(B_unreached);
	exit(B_ecount);
}

void
exec(void)
{
	B_search = Dfs;
	get_stack = dfs_stacktrace;
	store_init(0, 0);
	start_search();
	print_state("search completed", 0);
}
