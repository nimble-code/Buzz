#include "buzz.h"
#include "y.tab.h"	// for TIMEOUT

#include "dfs_common.c"

static int
has_successor(short timeout_enabled)
{	short n, t, po_safe;
	short stopped = 0, executed = 0;

	for (n = 0; n < B_nproc; n++)	// all processes
	{	B_procname = B_pnames[n]->nm;
		if (!B_state.s[n])
		{	stopped++;
			if (B_verbose)
			{	printf("\tproc %s is stopped\n", B_procname);
			}
			continue;
		}
		stack[depth].n  = n;
		po_safe = (B_state.s[n]->alt[0]->tag & PO_Safe);
		for (t = 0; t < Fmax && B_state.s[n]->alt[t]; t++) // all transitions
		{	AST *a = B_state.s[n]->alt[t];
			B_pid = n;
			B_procname = B_pnames[n]->nm;
			a->tag |= Reached;

			if ((timeout_enabled && a->tok == TIMEOUT)
			||  step(a, executed))
			{	executed++;
				stack[depth].t  = t;
				B_state.s[n] = a->nxt;
				dfs();
				memcpy( (void *) &B_state,
					(void *) stack[depth].b,
					B_ssize);
		}	}
		if (executed && po_safe)
		{	break;
	}	}

	if (B_verbose)
	{	printf("executed %d stopped %d (timeouts: %d)\n",
			executed, stopped, timeout_enabled);
	}

	return (executed || stopped == B_nproc);
}

static void
dfs(void)
{
	depth++;
	if (B_verbose)
	{	printf("%3ld: Down (%d)\n", depth, warned);
	}
	if (depth > mdepth)
	{	mdepth = depth;
	}
	if (depth >= B_maxdepth)
	{	if (!warned)
		{	printf("max depth reached (-m%3ld)\n", depth);
			warned = 1;
		}
		goto up;
	} else if (store_state((uchar *) &B_state, B_ssize) == New)
	{
		if (B_verbose)
		{	printf("\tnew state\n");
		}
		if (((++B_nstates)%(1000*B_freq)) == 0)
		{	dfs_stats(0);
		}

		stack[depth].b = get_last_state();

		if (!has_successor(NO_TIMEOUT)
		&&  !has_successor(USE_TIMEOUT)
		&&  ++B_ecount == B_ewant)
		{	print_state("deadlock state", 1);
		}
	} else
	{	B_ntruncs++;
		if (B_verbose)
		{	printf("\told state\n");
	}	}
up:
	if (B_verbose)
	{	printf("%3ld: Up (%d)\n", depth, warned);
	}
	depth--;
}

void
start_search(void)
{
	if (B_mode == Bitstate || B_mode == Twobit)
	{	fatal("store=bitstate requires search=dfs_bit or search=dfs_bit_claim");
	}
	stack = (Stack *) e_malloc((B_maxdepth)*sizeof(Stack));
	dfs();
}
