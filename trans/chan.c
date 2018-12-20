#include "buzz.h"
#include "y.tab.h"

typedef struct Channels Channels;
static struct Channels {
	char     *cname;
	int	  cnr;
	Channels *nxt;
} *channels;

static int	 nchan;

int
find_chan(char *s)
{	Channels *c;

	for (c = channels; c; c = c->nxt)
	{	if (strcmp(c->cname, s) == 0)
		{	return c->cnr;
	}	}

	c = (Channels *) e_malloc(sizeof(Channels));
	c->cnr = nchan++;
	c->cname = (char *) e_malloc(strlen(s)+1);
	strcpy(c->cname, s);
	c->nxt = channels;
	channels = c;

	if (nchan >= SV)
	{	fatal("too many channels, increase SV");
	}

	return c->cnr;
}

// return 0 if unexecutable, with no side-effects
// return 1 if executed

#define Assert(e)	assert(e)

int
transition(AST *a, int notblocked)
{	int dst = 0, msg = -1, rv = 0;

	Assert(a);

	switch (a->tok) {
	case '!':	// dst [!?] msg
		Assert(a->attr && a->attr->attr);
		dst = a->attr->val;
		msg = a->attr->attr->val;
		Assert(dst >= 0 && dst < SV && msg > 0);
		if (B_state.sv[dst] == 0)
		{	B_state.sv[dst] = msg;
			rv = 1;
		}
		break;
	case '?':
		Assert(a->attr && a->attr->attr);
		dst = a->attr->val;
		msg = a->attr->attr->val;
		Assert(dst >= 0 && dst < SV && msg > 0);
		if (B_state.sv[dst] == msg)
		{	B_state.sv[dst] = 0;
			rv = 1;
		}
		break;
	case '@':
		Assert(a->attr && a->attr->attr);
		rv = proc_at_label(a);
		break;
	case TIMEOUT:
	case TRUE:
		rv = 1;
		break;
	case FALSE:
		break;
	case ELSE:
		rv = (notblocked == 0);
		break;
	default:
		fatal("internal error: unknown token, function transition");
		break;
	}

	if (B_verbose && rv)
	{	fprintf(stderr, "\t%s:%3d: proc %s exec [%d,%d] ",
			a->fnm, a->ln, B_procname, dst, msg);
		trans_print(a);
	}

	return rv;
}

static char *
chan_name(int n)
{	Channels *c;
	int i = nchan-1;

	for (c = channels; c; c = c->nxt, i--)
	{	if (i == n)
		{	return c->cname;
	}	}
	return "unknown";
}

void
trans_print(AST *a)
{
	if (a)
	switch (a->tok) {
	case '!':
	case '?':
		assert(a && a->attr->attr);	// msg
		fprintf(stderr, "%s%c%s\n",
			chan_name(a->attr->val),			// dst
			(char) a->tok,
			reverse_lookup(a->attr->attr->val - 1));	// msg
		break;
	case NEXT:
		fprintf(stderr, "next %s\n", a->attr->nm);
		break;
	case '@':
	case TRUE:
	case FALSE:
	default:
		fprintf(stderr, "%s\n", a->nm);
		break;
	}
}

void
trans_stats(void)
{
	fprintf(stderr, "#chans %d\n", nchan);
}

void
chan_state(Channels *c, int i)
{
	if (!c || i < 0)
	{	return;
	}
	chan_state(c->nxt, i-1);

	if (B_state.sv[i])
	{	fprintf(stderr, "\tchan %s: %s\n",
			c->cname, reverse_lookup(B_state.sv[i] - 1));
	}
}

void
trans_state(void)
{
	chan_state(channels, nchan-1);
}
