#include "buzz.h"
#include "y.tab.h"
#include "functions.h"

// return 0 if unexecutable, with no side-effects
// return 1 if executed

Fct *functions;

int
find_fct(AST *a)
{	int i;

	if (functions)
	for (i = 0; functions[i].f; i++)
	{	if (strcmp(functions[i].f, a->nm) == 0)
		{	return i;
	}	}
	fprintf(stderr, "cannot find user-defined function '%s'\n", a->nm);
	exit(1);
}

int
transition(AST *a, int notblocked)
{	int rv = 0;

	assert(a);

	switch (a->tok) {
	case ELSE:
			rv = (notblocked == 0);
			break;
	case '@':
			assert(a->attr && a->attr->attr);
			rv = proc_at_label(a);
			break;
	case TIMEOUT:
	case TRUE:
			rv = 1;
			break;
	case FALSE:
			rv = 0;
			break;
	default:
			if (a->val > 0)	// function call
			{  rv = functions[a->val-1].fct();
			}
			break;
	}

	if (B_verbose)
	{	fprintf(stderr, "\t%s:%3d: proc %s exec %s (tok %d) = %d\n",
			a->fnm, a->ln, B_procname, a->nm, a->tok, rv);
	}

	return rv;
}

void
trans_print(AST *a)
{
	assert(a->nm);
	fprintf(stderr, "%s()\n", a->nm);
}

void
trans_stats(void)
{
	fprintf(stderr, "\n");
}

void
trans_state(void)
{
}
