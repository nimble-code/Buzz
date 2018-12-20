
typedef struct Addr Addr;
static struct Addr {
	void *a;
	Addr *nxt;
} *addr;

int
statenr(void *a)
{	Addr *b;
	int cnt;

	for (b = addr, cnt=0; b; b = b->nxt, cnt++)
	{	if (b->a == a)
		{	return cnt;
	}	}

	b = (Addr *) e_malloc(sizeof(Addr));
	b->a = a;
	b->nxt = addr;
	addr = b;
	return cnt;
}

void
dump_ast(AST *a, int pref)
{	int i;

	if (a && !(a->tag & Tagged))
	{	a->tag |= Tagged;
		for (i = 0; i < pref; i++)
		{	printf("\t");
		}
		if (a->tok == PROC)
		{	printf("%s:%3d: proc %s -- startstate S%d\n",
				a->fnm, a->ln, a->nm, statenr((void *) a->attr));
		} else
		{	printf("%s:%3d: S%d\ttok = %d '%s' nxt S%d",
				a->fnm, a->ln, statenr((void *) a),
				a->tok, a->nm,
				statenr((void *) a->nxt));
			for (i = 0; i < Fmax && a->alt[i]; i++)
			{	printf(" alt[%d] S%d", i, statenr((void *) a->alt[i]));
		}	}

		printf("\n");
		if (a->tok == PROC)
		{	dump_ast(a->attr, 1);
		}
		for (i = 0; i < Fmax && a->alt[i]; i++)
		{	dump_ast(a->alt[i], pref);
		}
		dump_ast(a->nxt, pref);
	}
}

void
fix_remote(AST *a)
{	int t;
	// runs after prep_ast, to resolve @

	if (a && !(a->tag&Visited))
	{	a->tag |= Visited;
		for (t = 0; t < Fmax && a->alt[t]; t++)
		{	fix_remote(a->alt[t]);
		}
		fix_remote(a->nxt);
		if (a->tok == PROC)
		{	fix_remote(a->attr);
		}
		if (a->nxt && a->nxt->tok == NEXT)
		{	a->nxt = a->nxt->nxt;
			a->nxt->tag |= Reached;
		}
		if (a->tok == '@')
		{	char *pnm;
			int   n;

			assert(a->attr && a->attr->attr);
			pnm = a->attr->nm;
		
			for (n = 0; n < B_nproc; n++)
			{	if (strcmp(B_pnames[n]->nm, pnm) == 0)
				{	a->val = 1+n;
					break;
			}	}
			if (n >= B_nproc)
			{	fprintf(stderr, "looking for: %s\n", pnm);
				fatal("process name in @ not found");
	}	}	}
}

void
prep_ast(AST *a)
{
	if (a)	// resolve 'next'
	{	prep_ast(a->nxt);
		if (a->tok == PROC)
		{	if (B_nproc >= MaxProc)
			{	fatal("too many processes, change MaxProc");
			}
			B_pnames[B_nproc]  = a;	  // set process name
			B_state.s[B_nproc] = a->attr; // initial state
			B_state.s[B_nproc]->tag |= Reached; // initial state

			B_nproc++;

			B_procname = a->nm;
			prep_ast(a->attr);
			B_procname = "-";
			a->val = (strncmp(a->nm, "claim", strlen("claim")) == 0);
			B_nmon += a->val;
			a->alt[0] = a;
		} else if (a->tok == NEXT)
		{	assert(a->attr);
			a->nxt = find_label(a->attr->nm);
			a->tag |= (Reached | PO_Safe);
			a->alt[0] = a->nxt;
		} else
		{	a->alt[0] = a;
			if (B_safe
			&&  strncmp(state_label(a), "safe_", strlen("safe_")) == 0)
			{	a->tag |= PO_Safe;
	}	}	}
}

int
proc_at_label(AST *a)
{	char *lnm;
	int n;

	if (a->val > 0)
	{	assert(a->val <= B_nproc);
		lnm = a->attr->attr->nm;
		n   = a->val - 1;

		return state_at(B_state.s[n], lnm);
	}
	return 0; // cannot really happen
}
