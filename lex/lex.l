%{
#include "buzz.h"
#include "y.tab.h"

static int linenumber = 1;
static int new_node(int);
%}

%option noyywrap
%option nounput
%option noinput

DIGIT           [0-9]
TEXT            [^"]*
IDENTIFIER      [A-Za-z_][A-Za-z0-9_]*

%%
" "|"\t"        ;
\n              linenumber++;
\/\/.*\n        linenumber++;
"else"		return new_node(ELSE);
"false"		return new_node(FALSE);
"process"	return new_node(PROC);
"next"		return new_node(NEXT);
"timeout"	return new_node(TIMEOUT);
"true"		return new_node(TRUE);
"%%"		return EOF;
{IDENTIFIER}    return new_node(NAME);
{DIGIT}+        return new_node(NUMBER);
.               return new_node(yytext[0]);
%%

static int
new_node(int tok)
{
	yylval = (AST *) e_malloc(sizeof(AST));
	assert(yylval);

	yylval->nm = (char *) e_malloc(strlen(yytext)+1);
	assert(yylval->nm);
	strcpy(yylval->nm, yytext);

	if (tok == NUMBER)
	{	yylval->val = atoi(yytext);
	}
	yylval->ln  = linenumber;
	yylval->fnm = B_filename;
	yylval->tok = tok;

	if (0)
	{	fprintf(stderr, "saw: '%s' (%d)\n", yytext, tok);
	}
	return tok;
}

void
yyerror(char *s)
{
        printf("%s:%d: %s (%s)\n", B_filename, linenumber, s, yytext);
	exit(1);
}
