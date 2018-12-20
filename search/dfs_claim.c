#include "buzz.h"
#include "y.tab.h"

#include "dfs_common.c"

static void async(void);

static void
dfs(void)	// claim move
{	short n, t, stopped, executed;

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
	} else if (store_state((uchar *) &B_state, sizeof(State)) == New)
	{	stopped = executed = 0;

		if (B_verbose)
		{	printf("\tnew state\n");
		}
		B_nstates++;
		if ((B_nstates%(1000*B_freq)) == 0)
		{	dfs_stats(0);
		}

		stack[depth].b = get_last_state();

		for (n = 0; n < B_nproc; n++)	// all monitors
		{	if (!B_pnames[n]->val)
			{	continue;
			}
			B_procname = B_pnames[n]->nm;
			if (!B_state.s[n])
			{	stopped++;
				if (++B_ecount == B_ewant)
				{	print_state("endstate in claim", 1);
				}
				break;
			}
			stack[depth].n  = n;

			for (t = 0; t < Fmax && B_state.s[n]->alt[t]; t++) // all transitions
			{	AST *a = B_state.s[n]->alt[t];
				B_pid = n;
				a->tag |= Reached;
				if (step(a, executed))
				{	executed++;
					stack[depth].t  = t;
					B_state.s[n] = a->nxt;
					async();	// process move
					memcpy(	(void *) &B_state,
						(void *) stack[depth].b,
						sizeof(State));
		}	}	}

		if (B_verbose)
		{	printf("executed %d stopped %d\n", executed, stopped);
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

static void
async(void)	// process move
{	int n, t, stopped, executed, po_safe;
	int timeout_enabled;

	depth++;
	if (B_verbose)
	{	printf("%3ld: Down (%d)\n", depth, warned);
	}

	if (depth > mdepth)
	{	mdepth = depth;
	}

	if (depth >= B_maxdepth)
	{	if (!warned)
		{	printf("%3ld: max depth reached (%d)\n", depth, warned);
			warned = 1;
		}
	} else	// 2nd half of a claim/proc pair
	{	stopped = executed = timeout_enabled = 0;

		if (!stack[depth].b)	// intermediate state not in hash-table
		{	stack[depth].b = (State *) e_malloc(sizeof(State));
		}

		memcpy(	(void *) stack[depth].b,
			(void *) &B_state, sizeof(State));

repeat:		for (n = 0; n < B_nproc; n++)	// all processes
		{	if (B_pnames[n]->val)	// synchronous monitor
			{	continue;
			}
			B_procname = B_pnames[n]->nm;
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
				a->tag |= Reached;
				if ((timeout_enabled && a->tok == TIMEOUT)
				||  step(a, executed))
				{	executed++;
					stack[depth].t  = t;
					B_state.s[n] = a->nxt;
					dfs();	// claim move
					memcpy(	(void *) &B_state,
						(void *) stack[depth].b,
						sizeof(State));
			}	}
			if (executed && po_safe)
			{	break;
		}	}

		if (B_verbose)
		{	printf("executed %d stopped %d\n", executed, stopped);
		}

		if (!executed && stopped < B_nproc - B_nmon)
		{	if (!timeout_enabled)
			{	timeout_enabled = 1;
				goto repeat;
			}
			if (++B_ecount == B_ewant)
			{	print_state("deadlock state", 1);
	}	}	}
	if (B_verbose)
	{	printf("%3ld: Up (%d)\n", depth, warned);
	}
	depth--;
}

void
start_search(void)
{
	stack = (Stack *) e_malloc((B_maxdepth)*sizeof(Stack));
	dfs();
}
