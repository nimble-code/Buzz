//  Buzz -- a basic model checker interface
//  (c) 2015 Gerard J. Holzmann
//  California Institute of Technology, Pasadena, CA, USA.
//
//  This is a teaching tool created for use in Caltech Course
//  CS118: Logic Model Checking for Formal Software Verification 
//
//  Buzz is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "buzz.h"
#include <signal.h>
#include <sys/times.h>
#include <unistd.h>
#include "y.tab.h"

static char *progname = "buzz";
static struct tms start_tm;
static struct tms stop_tm;
static clock_t start_time;
static clock_t stop_time;
static double delta_time;
static ulong B_mem_used;	// dynamic allocation

AST	*B_ast;			// abstract syntax tree
AST	*B_pnames[MaxProc];	// process names

State	 B_state;		// global system state
StorageMode B_mode = Closed;	// default storage mode
SearchMode  B_search = Dfs;	// default search mode
Parser	 B_parse = Function_mode; // default

char	*B_procname = "-";	// name current process
char	*B_filename;		// name current file

ulong	 B_ssize = sizeof(State); // main is always recompiled
ulong	 B_nstates;		// nr states reached
ulong	 B_ntruncs;		// nr states revisited
ulong	 B_maxdepth = 10000;	// default maximum search depth
ulong	 B_maxgen   = 10000;	// max size of a bfs generation, in thousands of states

int	 B_ecount;		// nr of errors found
int	 B_ewant = 1;		// want first error
int	 B_freq = 1000;		// reporting frequency in thousands, during searches
int	 B_maxtry = 20;		// max nr of rehashes in closed hashing
int	 B_ncores = 8;		// default nr of cores in multi-core searches
int	 B_nlocks = 1;		// minimally one
int	 B_nmon;		// nr of monitors
int	 B_nproc;		// nr of processes
int	 B_parmem = 24;		// nr of GB of shared memory to use
int	 B_pid;			// current process
int	 B_safe;		// non-zero if there are any po_safe states
int	 B_test;		// if set, do a store_test at the end, enabled with -T
int	 B_terse;		// no unvisited state listing if set
int	 B_unreached;		// nr of unreached fsm states
int	 B_verbose;		// debugging
int	 B_width = 20;		// 2^size of hashtable or bitstate array

void   (*get_stack)(ulong, long);
State *(*get_last_state)(void);
ulong  (*get_parent)(ulong);
void   (*set_parent)(ulong, ushort, ushort);
ushort (*get_last_pid)(void);
ushort (*get_last_trans)(void);
void (*e_mutex)(int);
void (*x_mutex)(int);
extern void *sbrk(intptr_t);

static void
usage(void)
{
	fprintf(stderr, "\t-V	print version info\n");
	fprintf(stderr, "\t-v	verbose\n");
	fprintf(stderr, "\t-cN	stop at Nth error\n");
	fprintf(stderr, "\t-f s	set filename to s\n");			//
	fprintf(stderr, "\t-mN	set max depth\n");
	fprintf(stderr, "\t-MN	set max shared memory (Gb)\n");	
	fprintf(stderr, "\t-qN	set max queue size bfs (K)\n");
	fprintf(stderr, "\t-rN	set reporting frequency (K)\n");
	fprintf(stderr, "\t-uN	set nr of cores to use\n");
	fprintf(stderr, "\t-wN	set hashtable size\n");
	fprintf(stderr, "\t-tN	set max tries for closed hashing\n");
	fprintf(stderr, "\t-T	give stats on storage\n");
	exit(1);
}
#ifdef USE_MALLOC
	#define Malloc(n)	malloc(n)
#else

#ifndef CHUNK_SZ
	#define CHUNK_SZ	1024*1024
#endif

void *
Malloc(size_t n)	// faster
{	static void *pool = 0;
	static ulong have = 0;
	void *give;

	if (have < n)
	{	have = (n < CHUNK_SZ) ? CHUNK_SZ : n;
		pool = sbrk((intptr_t) have);
		if (pool == (void *) -1)
		{	fprintf(stderr, "buzz: tried to allocate %lu bytes\n",
				have);
			fatal("Malloc: not enough memory");
	}	}
	give  = pool;
	have -= n;
	pool  = (void *) ((uchar *) pool + n); // avoids gcc warning

	return give;
}
#endif

void *
e_malloc(size_t n)
{	void *v;

	while (n%WS) { n++; }

	v = (void *) Malloc((size_t) n);	// was malloc
	if (!v)
	{	fatal("e_malloc: out of memory");
	}
	memset(v, 0, (size_t) n);
	B_mem_used += (ulong) n;
	return v;
}

ulong
mem_MB_used(void)
{
	return B_mem_used/(1024*1024);
}

void
fatal(char *s)
{
	fprintf(stderr, "error: %s\n", s);
	record_time();
	report_time(0);
	exit(1);
}

static void
stopped(int arg)
{
	print_state("interrupt", 0);
	exit(1);
}

void
record_time(void)
{
	stop_time  = times(&stop_tm);
	delta_time = ((double) (stop_time - start_time)) / ((double) sysconf(_SC_CLK_TCK));
}

void
report_time(int lf)
{
	fprintf(stderr, "%s\telapsed %.3g sec, ",
		lf?"\n":"", delta_time);
	if (delta_time > 0.01)
	{	fprintf(stderr, "rate %.6g states/sec\n",
			B_nstates/delta_time);
	} else
	{	fprintf(stderr, "\n");
	}
}

// stubs to allow compilations to succeed
void   no_trace(ulong unused1, long unused2) {}
ulong  no_parent(ulong unused)	{ return 0; }
void   not_set(ulong unused1, ushort unused2, ushort unused3) {}
void   not_critical(int unused) {}
ushort no_short(void) { return 0; }

State *no_laststate(void)
{
	fatal("laststate function undefined in this mode");
	return 0;
}

int
main(int argc, char *argv[])
{
	get_stack      = no_trace;
	get_last_state = no_laststate;
	get_last_pid   = no_short;
	get_last_trans = no_short;
	get_parent     = no_parent;
	set_parent     = not_set;
	e_mutex        = not_critical;
	x_mutex        = not_critical;

	while (argc > 1 && argv[1][0] == '-')
	{	switch (argv[1][1]) {
		case 'V': printf("%s\n", Version); exit(0);
		case 'v': B_verbose++; break;
		case 'c': B_ewant = atoi(&argv[1][2]); break;
		case 'f': B_filename = argv[2]; argc--; argv++; break; // internal
		case 'm': B_maxdepth = atoi(&argv[1][2]); break;
		case 'n': B_terse++; break;
		case 'M': B_parmem   = atoi(&argv[1][2]); break;
		case 'q': B_maxgen   = (ulong) atoi(&argv[1][2]) * 1000UL; break;
		case 'r': B_freq     = atoi(&argv[1][2]); break;
		case 'u': B_ncores   = atoi(&argv[1][2]); break;
		case 'w': B_width    = atoi(&argv[1][2]); break;
		case 't': B_maxtry   = atoi(&argv[1][2]); break;
		case 'T': B_test++; break;
		default:  usage(); break;
		}
		argv++; argc--;
	}
	if (argc != 2)
	{	usage();
	}
	if (!B_filename)
	{	B_filename = argv[1];
	}
	yyin = fopen(argv[1], "r");
	if (!yyin)
	{	fprintf(stderr, "%s: cannot find '%s'\n",
			progname, argv[1]);
		exit(1);
	}
	assert(yyin);

	init();
	(void) yyparse();
	fclose(yyin);
	assert(B_ast);
	start_time = times(&start_tm);
	signal(SIGINT, stopped);

	exec();

	return B_ecount;
}

static void
do_unvisited(AST *a)
{	int i;

	if (a && !(a->tag&Marked))
	{	a->tag |= Marked;
		do_unvisited(a->nxt);
		if (a->tok == PROC)
		{	do_unvisited(a->attr);
		} else
		{	if (!(a->tag&Reached))
			{	if (!B_terse)
				{	fprintf(stderr, "\t%s:%d:\tunreached: ",
						a->fnm, a->ln);
					trans_print(a);
				}
				B_unreached++;
			} else if (B_verbose && !B_terse)
			{	printf("\t%s:%d:\t'%s'\treached\n",
					a->fnm, a->ln, a->nm);
			}
			for (i = 0; i < Fmax; i++)
			{	if (!a->alt[i])
				{	break;
				} else
				{	do_unvisited(a->alt[i]);
	}	}	}	}
}

void
unvisited(AST *a)
{
	fprintf(stderr, "unreached:\n");
	do_unvisited(a);
	fprintf(stderr, "\t(%d states unreached)\n", B_unreached);
}

int
step(AST *a, int m)
{
	if (!a)
	{	return 0;
	}

	if (a->tok == NEXT)
	{	if (B_verbose>1)
		{	assert(B_procname);
			fprintf(stderr, "\t%s:%3d: proc %s exec %s\n",
				a->fnm, a->ln, B_procname, a->nm);
		}
		return 1;
	}

	return transition(a, m);
}

const char *
store_mode(void)
{
	switch (B_mode) {
	case Open:		return "Open";
	case Open_par:		return "Open_par";
	case OpenCompact:	return "OpenCompact";
	case OpenCompact_par:	return "OpenCompact_par";
	case Compact:		return "Compact";
	case Compact_par:	return "Compact_par";
	case Closed:		return "Closed";
	case Closed_notrace:	return "Closed_notrace";
	case Closed_par:	return "Closed_par";
	case Bitstate:		return "Bitstate";
	case Twobit:		return "Twobit";
	default:		break;
	}
	return "Unknown";
}

const char *
search_mode(void)
{
	switch (B_search) {
	case Dfs:		return "Dfs";
	case Dfs_bit:		return "Dfs_bit";
	case Bfs:		return "Bfs";
	case Bfs_notrace:	return "Bfs_notrace";
	case Bfs_toggle:	return "Bfs_toggle";
	case Bfs_par:		return "Bfs_par";
	case Bfs_par_q:		return "Bfs_par_q";
	}
	return "Unknown";
}
