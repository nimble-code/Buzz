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

#ifndef BATS_INCLUDE
#define BATS_INCLUDE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

// version 0.0 was created Nov. 28, 2013

#define Version	"Version 0.2 -- February 20, 2015"
#define WS	sizeof(void *)

typedef unsigned int	uint;
typedef unsigned short	ushort;
typedef unsigned long	ulong;
typedef unsigned char	uchar;

// Interface definitions
// 1. Parser and Lexical Analyzer: parse.y, lex.l

	#ifndef Fmax
		#define Fmax	32	// max # successors
	#endif
	#ifndef MaxProc
		#define MaxProc	16	// max # processes
	#endif

	#define YYSTYPE	AST*
	#define YYMAXDEPTH	160000	// default is 10000

	typedef enum StateTag {
		Visited=1, Reached=2, Marked=4, Tagged=8, PO_Safe=16
	} StateTag;

	typedef enum Parser {
		Function_mode, Channel_mode
	} Parser;

	typedef struct AST {
		int	 tok;	// type
		char	*nm;	// name
		int	 val;	// value
		char	*fnm;	// filename
		int	 ln;	// linenumber
		uint	 tag;	// StateTag

		struct AST *attr;
		struct AST *nxt;

		struct AST *alt[Fmax]; // outgoing transitions
	} AST;

	extern FILE *yyin;
	extern int   yylex(void);
	extern void  yyerror(char *);
	extern int   yyparse(void);
	extern int   fileno(FILE *); // POSIX but not ISO C
	extern void  prep_ast(AST *);
	extern void  fix_remote(AST *);

// 2. Step Semantics: trans.c

	extern void dump_ast(AST *, int);
	extern void trans_print(AST *);
	extern void trans_stats(void);
	extern void trans_state(void);
	extern int  find_chan(char *);		// parse.y
	extern int  find_fct(AST *);		// fct.y
	extern int  proc_at_label(AST *);
	extern int  transition(AST *, int);

// 3. Symbol Table: sym.c

	extern void  add_label(char *, AST *);
	extern AST  *find_label(char *);
	extern int   find_sym(char *);
	extern char *reverse_lookup(int);
	extern char *state_label(AST *);
	extern int   state_at(AST *, char *);

// 4. Hash: hash.c

	extern uint64_t hash(uchar *, int);
	extern uint64_t hash_s(uchar *, int, uint64_t); // with seed

// 5. Store: store.c

	#include "state.h"

	typedef enum Results {
		Unknown, OnStack, New, Old
	} Results;

	typedef enum StorageMode {
		Open,
		Open_par,
		OpenCompact,
		OpenCompact_par,
		Compact,
		Compact_par,
		Closed,
		Closed_notrace,
		Closed_par,
		Bitstate,
		Twobit
	} StorageMode;

	typedef enum SearchMode {
		Dfs,
		Dfs_bit,
		Bfs,
		Bfs_notrace,
		Bfs_toggle,
		Bfs_par,
		Bfs_par_q
	} SearchMode;

	extern Results	store_state(uchar *, int);
	extern State   *store_last_state(void);
	extern ulong	store_get_parent(ulong);
	extern ulong	store_init(uchar *, ulong);
	extern ulong	store_last_slot(void);
	extern ushort	store_get_pid(void);
	extern ushort	store_get_trans(void);
	extern void	set_last_slot(ulong);
	extern void	store_full(void);
	extern void	store_set_parent(ulong, ushort, ushort);
	extern void	store_stats(void);
	extern void	store_test(void);

// 6. Search: dfs.c/bfs.c etc.

	#ifndef Mwait
		#define Mwait	(1<<20)
	#endif

	#define NO_TIMEOUT	0
	#define USE_TIMEOUT	1

	extern void init(void);	// auto-generated
	extern void exec(void);
	extern void start_search(void);
	extern void print_state(const char *, int);

// 7. Abstraction: tbd

// 8. Stack Functions: tbd

// Data:
//	all data are declared file static
//	except for these globals declared in main.c:

	extern AST  *B_ast;
	extern AST  *B_pnames[MaxProc];

	extern State       B_state;
	extern StorageMode B_mode;
	extern SearchMode  B_search;
	extern Parser	   B_parse;

	extern char *B_procname;
	extern char *B_filename;

	extern ulong B_ssize;	 // size of a state
	extern ulong B_maxdepth;
	extern ulong B_maxgen;	 // bfs
	extern ulong B_nstates;
	extern ulong B_ntruncs;

	extern int   B_ecount;
	extern int   B_ewant;
	extern int   B_freq;
	extern int   B_maxtry;	 // closed
	extern int   B_ncores;	 // parallel
	extern int   B_nlocks;	 // parallel
	extern int   B_nmon;	 // claim
	extern int   B_nproc;
	extern int   B_parmem;	 // parallel
	extern int   B_pid;
	extern int   B_safe;
	extern int   B_test;
	extern int   B_unreached;
	extern int   B_verbose;
	extern int   B_width;	 // non-bitstate

// Utility Functions:

	extern State *(*get_last_state)(void);
	extern ushort (*get_last_pid)(void);
	extern ushort (*get_last_trans)(void);
	extern ulong  (*get_parent)(ulong);
	extern void   (*set_parent)(ulong, ushort, ushort);
	extern void   (*get_stack)(ulong, long);
	extern void   (*e_mutex)(int);
	extern void   (*x_mutex)(int);

	extern const char *search_mode(void);
	extern const char *store_mode(void);

	extern void *e_malloc(size_t);
	extern void  fatal(char *);
	extern void  record_time(void);
	extern void  report_time(int);
	extern void  unvisited(AST *);

	extern int   statenr(void *);
	extern int   step(AST *, int);

	extern ulong mem_MB_used(void);

//	extensions are ideally implemented by adding
//	interface functions, not by adding globals
#endif	// BATS_INCLUDE
