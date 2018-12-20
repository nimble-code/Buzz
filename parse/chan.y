%{
#include "buzz.h"
%}

%start system

%token PROC NAME NUMBER NEXT ELSE TIMEOUT TRUE FALSE

%%
system	: model		{ B_parse = Channel_mode;
			  prep_ast(B_ast);
			  fix_remote(B_ast);
			  if (B_verbose==3)
			  {	dump_ast(B_ast, 0);
				exit(0);
			} }
	;

model	: proc		{ B_ast = $1; }
	| proc model	{ B_ast = $1; $1->nxt = $2; }
	;

proc	: PROC NAME      { B_procname = $2->nm; }
	  '{' seq '}'    { $$->nm = $2->nm; $$->attr = $5; }
	;

seq	: /* none */     { $$ = 0; }
	| stmnt ';' seq  { $1->nxt = $3; }
	;

stmnt	: NAME '!' NAME  { $$ = $2;
			   $$->attr = $1;
			   $1->attr = $3;
			   $1->val = find_chan($1->nm);
			   $3->val = find_sym($3->nm)+1;
			 }
	| NAME '?' NAME  { $$ = $2;
			   $$->attr = $1;
			   $1->attr = $3;
			   $1->val = find_chan($1->nm);
			   $3->val = find_sym($3->nm)+1;
			 }
	| NAME '@' NAME	 { $$ = $2;
			   $$->attr = $1;
			   $1->attr = $3;
			 }
	| ELSE | TIMEOUT | TRUE | FALSE
	| NAME ':' stmnt { $$ = $3; add_label($1->nm, $3); }
	| NEXT NAME      { $$->attr = $2; }
	;
%%

#include "parse/common.c"
