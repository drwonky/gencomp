%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*#include "sp_make.h"*/
#include "sp_parse.h"
#include "globals.h"
#include "symtab.h"

#define save_token(x) prlval.string=strdup(x)

%}

space	[ \t]+
input	"i"
output	"o"
result	"r"
comment	#.*
word	([A-Za-z0-9]|"_")+
string	["]([A-Za-z0-9]|"_"|" ")+["]
type	"type"
define	"define"
parm	"parm"
nl	\n+

%%

{comment}		;

^{define}		{
				return PR_DEF;
			}

^{parm}			{
				return PR_PARM;
			}

^{type}			{
				return PR_TYPE;
			}

^{input}		{
				return PR_INPUT;
			}

^{output}		{
				return PR_OUTPUT;
			}

^{result}		{
				return PR_RESULT;
			}

{word}			{
				save_token(prtext);
				return PR_WORD;
			}

{string}		{
				save_token(prtext);
				return PR_STRING;
			}

{space}		 	;

{nl}			lineno++;


%%

int prwrap(void)
{
	return 1;
}

/*
#define yyerror printf

void main(void)
{
	int	i;

	while(i=prlex()) {
		printf("%s -> %d\n",prtext,i);
	}
}
*/
