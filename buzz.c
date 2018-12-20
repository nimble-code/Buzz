//  Buzz -- a basic model checker interface
//  (c) 2015-2019 Gerard J. Holzmann
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#define Version	"Buzz Version 1.0 -- 1 January 2019"

#ifndef BUZZ_DIR
	// default location of module .o files
	// and the source tree of all modules
	// (in some cases the .c file is used
	//  instead of the .o file)
	// specified in the makefile
	// can be changed with -L argument
	#error "no BUZZ_DIR is specified in the makefile"
#endif
#ifndef BUZZ_EXE
	// path to generated executable
	// by default this is the directory
	// in which buzz is invoked
	#define BUZZ_EXE "./buzz_e"
#endif

typedef enum Deps {
	Search=1,   Parse=2,        Trans=4,
	StoreAll=8, StoreClosed=16, Sym=32
} Deps;

static struct Dr {		// configurable options, via compiler directives
	char *nm, *dflt, *val, *expl;
	int   dependencies;	// modules that need recompiling if value changes
	char *flag;		// value is to be converted to a runtime flag
} directives[] = {
	// these options require recompilation of some parts of the code
	{ "SV",        "128", 0, "state vector size in uchar",        (Search|Parse|Trans), 0 },
	{ "MaxProc",    "16", 0, "max nr of process declarations",    (Search|Parse),       0 },
	{ "MaxChan",    "16", 0, "when using trans/trans",            (Trans),              0 },
	{ "Fmax",       "32", 0, "max successors per state",          (Search|Parse|Sym),   0 },

	// the remaining options do not require recompilations
	{ "MaxGen",     "10", 0, "max size of a sequential bfs gen in thousands", 0, "-q" },
	{ "MaxDepth","10000", 0, "max search depth",                     0, "-m" },
	{ "Freq",     "1000", 0, "reporting frequency in thousands",     0, "-r" },
	{ "Ncores",      "8", 0, "nr of cores for multicore",            0, "-u" },
	{ "SGB",	"24", 0, "nr of GB of shared mem for multicore", 0, "-M" },
	{ "Hmax",	"20", 0, "2^20 size of hash-table",	         0, "-w" },
	{ "Tmax",       "20", 0, "max nr of retries in closed-hash",     0, "-t" },
	{ 0, 0, 0, 0 }
};

static struct Tb {
	char *key, *dflt, *val;
	int  recompile;
} table[] = {
	{ "base",    BUZZ_DIR, 0, 0 },	// first 3 entries
	{ "lex",    "lex",     0, 0 },	//  must be 0,1,2
	{ "parse",  "parse",   0, 0 },	//  in this order
	{ "sym",    "sym",     0, 0 },
	{ "trans",  "trans",   0, 0 },
	{ "hash",   "jenkins", 0, 0 },
	{ "store",  "closed",  0, 0 },
	{ "search", "dfs",     0, 0 },
	{ 0, 0, 0, 0 }
	// main/main.c depends on y.tab.h for PROC and NEXT
	// and is therefore not configurable
};

typedef struct Names {
	char *nm;
	int   val;
	struct Names *nxt;
} Names;

Names *names;
Names *functions;
Names *statevars;
Names *inits;

static int verbose, terse, ewant=1, debug;
static int B_test, B_parmem;
static char *progname;
static char olevel = '3';
static int made_tmp;
extern int mkstemps(char *, int); // should also be stdlib.h

static void
add_init(char *s)
{	Names *n;

	n = (Names *) malloc(sizeof(Names));
	n->nm = (char *) malloc(strlen(s)+1);
	strcpy(n->nm, s);
	n->nxt = inits;
	inits = n;
}

static void
scan_dir(char *s, char *p, char *dflt)
{	DIR *d;
	struct dirent *e;
	char buf[512];
	char nm[64];
	int cnt;

	sprintf(buf, "%s/%s", p, s);
	if ((d = opendir(buf)) == NULL)
	{	fprintf(stderr, "%s: cannot opendir %s\n",
			progname, buf);
		exit(1);
	}
	printf("\t%s=%s\t", s, dflt);
	cnt = 0;
	while ((e = readdir(d)) != NULL)
	{	if (strstr(e->d_name, ".o"))
		{	strcpy(nm, e->d_name);
			nm[strlen(e->d_name)-2] = '\0';
			if (strcmp(nm, dflt) != 0)
			{	if (cnt++ == 4)
				{	cnt = 0;
					printf("\n\t\t\t");
				}
				printf("| %s ", nm);
	}	}	}
	printf("\n");
	closedir(d);
}

static void
usage(char *s)
{	int i;

	printf("%s\n", s);
	printf("usage: '%s [options | sizes | modules]* filename'\n", progname);
	printf("options:\n");
	printf("	-cN	print the Nth error and then stop\n");
	printf("	-d	increase debug level\n");
	printf("	-mN	maxdepth = N in steps	 default 10000\n");
	printf("	-MN	set max amount of shared memory to N Gb, default 24\n");
	printf("	-ON	use compiler optimization N  default -O3\n");
	printf("	-qN	set max queue size in breadth-first searches to N Kb, default 10\n");
	printf("	-rN	freq     = N in Kstates	 default 1000\n");
	printf("	-uN	ncores   = N in cores	 default 8\n");
	printf("	-V	print version and exit\n");
	printf("	-v	increase verbosity level\n");
	printf("	-wN	width    = N in 2^N	 default 20 (hashtable)\n");
	printf("	-tN	maxtry   = N in tries	 default 20\n");
	printf("	-T	give stats on storage\n\n");

	printf("sizes (with the default values):\n");
	for (i = 0; directives[i].nm; i++)
	{	printf("\t%8s=%s\t(%s)\n",
			directives[i].nm,
			directives[i].dflt,
			directives[i].expl);
	}

	printf("\nmodules (the first in each list, after the = sign, is the default):\n");
	for (i = 0; table[i].key; i++)
	{	if (i > 0)
		{	scan_dir(table[i].key, table[0].dflt, table[i].dflt);
		} else
		{	printf("\t%s=%s\t(where the .o files are)\n",
				table[0].key, table[0].dflt);
	}	}

	exit(1);
}

static int
recompile(FILE *fd, char *s, int f)
{	int i;

	if (debug)
	{	fprintf(stderr, "# recompile %s f=%d\n", s, f);
	}

	for (i = 1; table[i].key; i++)
	{	if (strcmp(table[i].key, s) == 0)
		{	table[i].recompile = f;
			return i;
	}	}

	if (debug
	&&  strcmp(s, "lex") != 0
	&&  strcmp(s, "parse") != 0)
	{	fprintf(stderr, "%s: warning, cannot find key '%s'\n",
			progname, s);
		for (i = 1; debug > 1 && table[i].key; i++)
		{	fprintf(stderr, "\thave:\t'%s'\n", table[i].key);
	}	}

	return 0;
}

static void
set_flags(FILE *fd, int dep)
{	int i;
	if (debug)
	{	fprintf(stderr, "# set_flags %d\n", dep);
	}
	if ((dep & Search) || statevars)
	{	(void) recompile(fd, "search", 1);
	}
	if ((dep & Parse) || statevars)
	{	i = recompile(fd, "parse", 1);
		if (i > 0)
		{	fprintf(fd, "yacc -v -d %s/%s/%s.y\n",
				table[0].val?table[0].val:table[0].dflt,
				table[i].key,
				table[i].val?table[i].val:table[i].dflt);
			table[i].key = "parse_tmp";
			fprintf(fd, "mv y.tab.c %s.c\n",
				table[i].val?table[i].val:table[i].dflt);
			made_tmp++;
		}
		i = recompile(fd, "lex", 1);
		if (i > 0)
		{	fprintf(fd, "lex -t %s/%s/%s.l > %s.c\n",
				table[0].val?table[0].val:table[0].dflt,
				table[i].key,
				table[i].val?table[i].val:table[i].dflt,
				table[i].val?table[i].val:table[i].dflt);
			table[i].key = "lex_tmp";
			made_tmp++;
		}
	}
	if ((dep & Trans) || statevars)
	{	(void) recompile(fd, "trans", 1);
	}
	if (dep & StoreAll)
	{	(void) recompile(fd, "store", 1);
	} else if (dep & StoreClosed)
	{	(void) recompile(fd, "store", 2);
	}
	if (dep & Sym)
	{	(void) recompile(fd, "sym", 1);
	}
}

static void
do_compilation(FILE *fd, char *fplugin)
{	int i;
	int cnt = 0;

	fprintf(fd, "cc -g -O%c -I$B -I$B/include -o %s \\\n\t",
		olevel, BUZZ_EXE);
	for (i = 0; directives[i].nm; i++)
	{	if (directives[i].val
		&& !directives[i].flag
		&&  strcmp(directives[i].val, directives[i].dflt) != 0)
		{	fprintf(fd, "-D%s=%s ",
				directives[i].nm,
				directives[i].val);
			cnt++;
	}	}
	if (cnt)
	{	fprintf(fd, "\\\n\t");
	}

	// i==0 is dir
	// i==1 is lex
	// i==2 is parse
	for (i = 1; table[i].key; i++)
	{	if (strstr(table[i].key, "_tmp"))	// lex or parse
		{  fprintf(fd, "./%s.c \\\n\t",
			table[i].val?table[i].val:table[i].dflt);
		} else
		{  fprintf(fd, "$B/%s/%s%s \\\n\t",
			table[i].key,
			table[i].val?table[i].val:table[i].dflt,
			table[i].recompile?".c":".o");		// PM recompile==2 is special
	}	}

	fprintf(fd, "$B/main/main.c \\\n");	// fixed
	fprintf(fd, "\t%s\n",
		(strlen(fplugin) > 0) ? fplugin : "$B/init/init.o");

}

static void
write_cmd(char *cmd, char *fspec, char *fplugin, char *orig)
{	FILE *fd;
	int i;

	if ((fd = fopen(cmd, "w")) == NULL)
	{	fprintf(stderr, "%s: cannot open '%s'",
			progname, cmd);
		exit(1);
	}

	fprintf(fd, "#!/bin/sh\n\n");
	fprintf(fd, "B=%s\n",
		table[0].val?table[0].val:table[0].dflt);

	// if there's a user-defined plugin, compile it
	if (strlen(fplugin) > 2)
	{	fprintf(fd, "cc -O%c -I$B/include -c %s\n", olevel, fplugin);
		fprintf(fd, "%srm -f %s\n", (debug)?"# ":"", fplugin);
		fplugin[strlen(fplugin)-1] = 'o'; // .c -> .o
	}

	// if values were changed, tag recompilation of dependent modules
	for (i = 1; directives[i].nm; i++)
	{	if (!directives[i].flag
		&&  (statevars
		||  (directives[i].val
		 &&  strcmp(directives[i].val, directives[i].dflt) != 0)))
		{	set_flags(fd, directives[i].dependencies);
	}	}

	do_compilation(fd, fplugin);

	fprintf(fd, "%s%s ", (debug)?"# ":"", BUZZ_EXE);

	if (ewant != 1)
	{	fprintf(fd, "-c%d ", ewant);
	}
	if (verbose)
	{	fprintf(fd, "-v ");
	}
	if (terse)
	{	fprintf(fd, "-n ");
	}
	if (B_test)
	{	fprintf(fd, "-T ");
	}
	if (B_parmem > 0)
	{	fprintf(fd, "-M%d ", B_parmem);
	}

	for (i = 1; directives[i].nm; i++)
	{	if (directives[i].flag
		&&  directives[i].val
		&&  strcmp(directives[i].val, directives[i].dflt) != 0)
		{	fprintf(fd, "%s%s ",
				directives[i].flag,
				directives[i].val);
	}	}

	if (names)	// spec defined constants
	{	fprintf(fd, "-f %s %s\n", orig, fspec);
	} else
	{	fprintf(fd, "%s\n", orig);
	}
	fprintf(fd, "x=$?\n");
	fprintf(fd, "%srm -f *.o %s _buzz_*\n", (debug)?"# ":"", BUZZ_EXE);
	if (made_tmp)
	{	fprintf(fd, "%srm -f ./parse.c ./lex.c ./chan.c\n", (debug)?"# ":"");
		fprintf(fd, "%srm -f ./y.output ./y.tab.h\n", (debug)?"# ":"");
	}
	fprintf(fd, "exit $x\n");
	fclose(fd);
	(void) chmod(cmd, 00700);
}

int
make_file(char *f, char *suff)
{
	sprintf(f, "_buzz_%s", suff);
	return creat(f, 00666);
}

static void
scan_names(char *p, Names **list)
{	Names *n;
	char *q;
	static int uniq;

	do {
		while (isblank((int) *p))
		{	p++;
		}
		q = p;
		while (!isspace((int) *q) && *q != '\0'
		&&     *q != ',' && *q != ';' && *q != '\n')
		{	q++;
		}
		*q++ = '\0';
		if (strlen(p) > 0)
		{	n = (Names *) malloc(sizeof(Names));
			n->nm = (char *) malloc(strlen(p)+1);
			strcpy(n->nm, p);
			n->val = uniq++;
			n->nxt = *list;
			*list = n;
			p = q;
		}
	} while (*p != '\n' && *p != '\0');

	if (debug)
	{	for (n = *list; n; n = n->nxt)
		{	printf("# Name %s = %d\n", n->nm, n->val);
	}	}
}

static void
add_function(char *s)
{	Names *n;
	static int uniq;

	for (n = functions; n; n = n->nxt)
	{	if (strcmp(n->nm, s) == 0)
		{	return;
	}	}
	n = (Names *) malloc(sizeof(Names));
	n->nm = (char *) malloc(strlen(s)+1);
	strcpy(n->nm, s);
	n->val = uniq++;
	n->nxt = functions;
	functions = n;
}

static void
scan_state(char *p, Names **list)
{	Names *n;
	char *q;

	if ((q = strchr(p, '\n')) != NULL)
	{	*q = '\0';
	}
	while (isblank((int) *p))
	{	p++;
	}
	if (strlen(p) > 0)
	{	n = (Names *) malloc(sizeof(Names));
		n->nm = (char *) malloc(strlen(p)+1);
		strcpy(n->nm, p);
		n->val = 0;
		n->nxt = *list;
		*list = n;
		p = q;
	}
}

static void
scan_function(char *p)
{	char in[512];
	char *q, *r;
	static int nomore = 0;

	assert(strlen(p) < sizeof(in));
	strcpy(in, p);

	if (nomore 
	||  strncmp(in, "%%", 2) == 0)
	{	nomore = 1;
		return;
	}

	for (r = in; ;)
	{	q = r = strstr(r, "()");
		if (!q)
		{	break;
		}
		*q = 0;
		q--;
		r++;
		while (isspace((int) *q))
		{	q--;
		}
		while (isalnum((int) *q) || *q == '_')
		{	q--;
		}
		add_function(++q);
	}
}

static void
replace_names(char *p)
{	char in[512], bf[32], *q = in, *z;
	Names *n;
	int cnt;

	assert(strlen(p) < sizeof(in));
	do {
		strcpy(in, p);
		*p = '\0';
		 q = in;
		for (n = names, cnt = 0; n; n = n->nxt)
		{	while ((z = strstr(q, n->nm)) != NULL)
			{	*z = '\0';
				strcat(p, q);
				sprintf(bf, "%d", n->val);
				strcat(p, bf);
				q = z+strlen(n->nm);
				cnt++;
		}	}
		strcat(p, q);
	} while (cnt > 0);
}

static char *
new_string(char *s)
{	char *p;

	p = (char *) malloc(strlen(s)+1);
	assert(p);
	strcpy(p, s);
	return p;
}

static void
set_require(char *s)
{	int i, j;

	if (strlen(s) <= 1)
	{	return;
	}

	if (debug)
	{	printf("# required: '%s'\n", s);
	}

	// command-line options override in-file options

	for (i = 0; directives[i].nm; i++)
	{	j = strlen(directives[i].nm);
		if (strncmp(s, directives[i].nm, j) == 0
		&&  s[j] == '=')
		{	if (!directives[i].val)
			{	directives[i].val = new_string(&s[j+1]);
			} else if (debug)
			{	printf("\t# suppressed\n");
			}
			return;
	}	}

	for (i = 0; table[i].key; i++)
	{	j = strlen(table[i].key);
		if (strncmp(s, table[i].key, j) == 0
		&&  s[j] == '=')
		{	if (!table[i].val)
			{	table[i].val = new_string(&s[j+1]);
			} else if (debug)
			{	printf("\t# suppressed\n");
			}
			return;
	}	}
	
	fprintf(stderr, "%s: option not recognized: %s\n",
		progname, s);
	exit(1);
}

static void
scan_requires(char *s)
{	char *q = s+strlen(s);
	char *options[ sizeof(directives) / sizeof(struct Dr) +
		       sizeof(table)      / sizeof(struct Tb) ];
	int m, n;

	for (n = 0; s < q && n < sizeof(options)/sizeof(char *); n++)
	{	while (isblank((int) *s))
		{	s++;
		}
		options[n] = s;	// start of option
		while (isalnum((int) *s) || *s == '=' || *s == '_')
		{	s++;
		}
		*s++ = '\0';	// end of option
	}
	if (n >= sizeof(options)/sizeof(char))
	{	fprintf(stderr, "too many requires (saw %d, max is %d)\n",
			n, (int) (sizeof(options)/sizeof(char)));
		exit(1);
	}
	for (m = 0; m < n; m++)
	{	set_require(options[m]);
	}
}

static void
e_write(int tfd, const char *s)
{
	if (write(tfd, s, strlen(s)) != strlen(s))
	{	fprintf(stderr, "write of '%s' failed\n", s);
		exit(1);
	}
}

static void
preprocess(char *s, char *tmp1, char *tmp2)
{	FILE  *fd;
	Names *n;
	char buf[256];
	int  tfd;
	int  lnr=1;
	int  i;

	if ((fd = fopen(s, "r")) == NULL)
	{	fprintf(stderr, "%s: cannot open %s\n",
			progname, s);
		exit(1);
	}
	tfd = make_file(tmp1, ".bb");
	if (tfd < 0)
	{	fprintf(stderr, "could not create temporary file .bb\n");
		exit(1);
	}
	while (fgets(buf, sizeof(buf), fd))
	{	if (strncmp(buf, "// constants", strlen("// constants")) == 0)
		{	scan_names(&buf[strlen("// constants")], &names);
		} else if (strncmp(buf, "// requires", strlen("// requires")) == 0)
		{	scan_requires(&buf[strlen("// requires")]);
		} else if (strncmp(buf, "// functions", strlen("// functions")) == 0)
		{	// do nothing
		} else if (strncmp(buf, "// statevar", strlen("// statevar")) == 0)
		{	scan_state(&buf[strlen("// statevar")], &statevars);
		} else if (strncmp(buf, "// init", strlen("// init")) == 0)
		{	add_init(&buf[strlen("// init")]);
		} else if (strncmp(buf, "%%", 2) == 0)
		{	close(tfd);
			scan_function(buf);
			tfd = make_file(tmp2, ".c");
			if (tfd < 0)
			{	fprintf(stderr, "error creating temp file\n");
				exit(1);
			}
			sprintf(buf, "#include \"buzz.h\"\n#include \"functions.h\"\n");
		} else
		{	replace_names(buf);
		}
		e_write(tfd, buf);
		scan_function(buf);
		memset(buf, 0, sizeof(buf));
		lnr++;
	}

	if (functions)
	{	e_write(tfd, "\nvoid\ninit(void) // generated\n{\n");

		for (n = inits; n; n = n->nxt)
		{	e_write(tfd, n->nm);
		}

		for (n = functions, i = 0; n; n = n->nxt)
		{	i++;
		}
		sprintf(buf, "\tfunctions = (Fct *) e_malloc(%d*sizeof(Fct));\n", i);
		e_write(tfd, buf);

		for (n = functions, i = 0; n; n = n->nxt, i++)
		{	sprintf(buf, "\tfunctions[%d] = (Fct) { \"%s\", %s };\n",
				i, n->nm, n->nm);
			e_write(tfd, buf);
			memset(buf, 0, sizeof(buf));
		}
		e_write(tfd, "}\n");
	} else
	{	set_require("parse=chan");
		set_require("trans=chan");
	}
	close(tfd);
	fclose(fd);
}

static void
last_first(FILE *fd, Names *n)
{
	if (n)
	{	last_first(fd, n->nxt);
		fprintf(fd, "\t%s\n", n->nm);
	}
}

static void
put_statevars(void)
{	FILE *fd;
	char where[128];

	sprintf(where, "%s/include/state.h",
		table[0].val?table[0].val:table[0].dflt);

	if ((fd = fopen(where, "w")) == NULL)
	{	fprintf(stderr, "bb: error: cannot create state.h\n");
		return;
	}

	if (!statevars)
	{	fprintf(fd, "#ifndef SV\n");
		fprintf(fd, "	#define SV	32\n");
		fprintf(fd, "#endif\n\n");
		fprintf(fd, "typedef struct State {\n");
		fprintf(fd, "\tuchar\tsv[SV];\n");
	} else
	{	fprintf(fd, "typedef struct State {\n");
		last_first(fd, statevars);
	}
	fprintf(fd, "\tAST\t*s[MaxProc];\n");
	fprintf(fd, "} State;\n");

	fclose(fd);
}

int
main(int argc, char *argv[])
{	char tmp1[32], tmp2[32], *ptr;

	progname = argv[0];
	memset(tmp2, 0, sizeof(tmp2));

	while (argc > 1)
	{	if (argv[1][0] == '-')
		{	switch (argv[1][1]) {
			case 'c':
				ewant = atoi(&argv[1][2]);
				break;
			case 'd':
				debug++;
				break;

			case 'L': // set new location for module library
				if (argv[1][2] != '\0')
				{	ptr = &argv[1][2];
				} else
				{	ptr = argv[2];
					argc--;
					argv++;
				}
				// overrides BUZZ_DIR
				table[0].dflt = malloc(strlen(ptr)+1);
				strcpy(table[0].dflt, ptr);
				printf("Module library set to : %s\n", ptr);
				break;
			case 'm':
				sprintf(tmp1, "MaxDepth=%d", atoi(&argv[1][2]));
				set_require(tmp1);
				memset(tmp1, 0, sizeof(tmp1));
				break;
			case 'M':
				if (!isdigit((int) argv[1][2]))
				{	usage("option requires number");
					break;
				}
				B_parmem = atoi(&argv[1][2]);
				break;
			case 'n':
				terse++;
				break;
			case 'O':
				if (!isdigit((int) argv[1][2]))
				{	usage("option requires number");
					break;
				}
				olevel = argv[1][2];
				break;
			case 'q':
				sprintf(tmp1, "MaxGen=%d", atoi(&argv[1][2]));
				set_require(tmp1);
				memset(tmp1, 0, sizeof(tmp1));
				break;
			case 'r':
				sprintf(tmp1, "Freq=%d", atoi(&argv[1][2]));
				set_require(tmp1);
				memset(tmp1, 0, sizeof(tmp1));
				break;
			case 't':
				sprintf(tmp1, "Tmax=%d", atoi(&argv[1][2]));
				set_require(tmp1);
				memset(tmp1, 0, sizeof(tmp1));
				break;
			case 'T':
				B_test++;
				break;
			case 'u':
				sprintf(tmp1, "Ncores=%d", atoi(&argv[1][2]));
				set_require(tmp1);
				memset(tmp1, 0, sizeof(tmp1));
				break;
			case 'v':
				verbose++;
				break;
			case 'V':
				printf("%s\n", Version);
				exit(0);
			case '-':
			case '?':
				usage(Version);
				break;
			case 'w':
				sprintf(tmp1, "Hmax=%d", atoi(&argv[1][2]));
				set_require(tmp1);
				memset(tmp1, 0, sizeof(tmp1));
				break;
			default:
				printf("saw: -%c, ", argv[1][1]);
				usage("unrecognized option");
				break;
			}
		} else if (strchr(argv[1], '=') != NULL)
		{	set_require(argv[1]);
		} else
		{	break;
		}
		argc--; argv++;
	}
	if (argc != 2)
	{	usage("missing filename");
	}

	preprocess(argv[1], tmp1, tmp2);

	write_cmd("_buzz_.sh", tmp1, tmp2, argv[1]);

	if (!names)
	{	if (system("rm -f ./_buzz_.bb") != 0)
		{	fprintf(stderr, "failed to remove _buzz_.bb\n");
	}	}

	put_statevars();

	if (debug)
	{	if (system("cat ./_buzz_.sh") != 0)
		{	fprintf(stderr, "unexpected failure\n");
			exit(1);
	}	}

	return system("./_buzz_.sh");
}
