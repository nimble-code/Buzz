#include "buzz.h"

typedef struct Stab {
	char *nm;
	int   nr;
	struct Stab *nxt;
} Stab;

typedef struct Lab {
	char *s;	// label name
	char *p;	// proc name
	AST  *a;	// statement
	struct Lab *nxt;
} Lab;

static Stab  *stab;
static Lab   *ltab;

int
find_sym(char *s)
{	Stab *t;
	static int seqnr = 0;

	for (t = stab; t; t = t->nxt)
	{	if (strcmp(t->nm, s) == 0)
		{	assert(t->nr < 256);	// to fit in a byte
			return t->nr;
	}	}

	t = (Stab *) e_malloc(sizeof(Stab));
	t->nm = s;
	t->nr = seqnr++;
	t->nxt = stab;
	stab = t;

	return t->nr;
}

char *
reverse_lookup(int i)
{	Stab *t;

	for (t = stab; t; t = t->nxt)
	{	if (t->nr == i)
		{	return t->nm;
	}	}
	return "-not-found-";
}

void
add_label(char *s, AST *a)	// used by parse.y only
{	Lab *t;
	int i;

	for (t = ltab; t; t = t->nxt)
	{	if (t->p == B_procname
		&&  t->a != NULL
		&&  strcmp(t->s, s) == 0)
		{	assert(t->a->alt[0]);
			for (i = 1; i < Fmax; i++)
			{	if (!t->a->alt[i])
				{	t->a->alt[i] = a;
					return;
			}	}
			fatal("too many successors, increase Fmax");
	}	}

	t = (Lab *) e_malloc(sizeof(Lab));
	t->s = s;
	t->p = B_procname;
	t->a = a;

	if (a->alt[0] != 0 && a->alt[0] != a)
	{	fatal("internal error, function add_label");
	}
	a->alt[0] = a;
	B_safe |= (strncmp(s, "safe_", strlen("safe_")) == 0);

	t->nxt = ltab;
	ltab = t;
}

AST *
find_label(char *s)	// used by parse.y only - label -> state
{	Lab *t;

	for (t = ltab; t; t = t->nxt)
	{	if (t->p == B_procname
		&&  strcmp(t->s, s) == 0)
		{	return t->a;
	}	}

	fprintf(stderr, "looking for: %s\n", s);
	fatal("no such state");

	return (AST *) 0;
}

char *
state_label(AST *a)
{	Lab *t;
	int i;

	for (t = ltab; t; t = t->nxt)
	{	if (t->a)
		for (i = 0; i < Fmax; i++)
		{	if (t->a->alt[i] == a)
			{	return t->s;
	}	}	}

	if (B_verbose>1)
	{	fprintf(stderr, "unlabeled state %s:%d\n",
			a?a->fnm:"", a?a->ln:-1);
	}
	return "_unknown_";
}

int
state_at(AST *a, char *s)	// process@label
{
	return (strcmp(state_label(a), s) == 0);
}
