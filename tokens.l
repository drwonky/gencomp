%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "template_tokens.h"
#include "globals.h"

#define save_token(x) yylval.string=strdup(x)
#define inc_lineno do { lineno++; } while(0)

/*
#undef save_token
#define save_token(x) do { printf("token(%s)\n",x); } while(0)
*/

/*
#define IF 100
#define ECHO 200
#define OPENBLOCK 300
#define CLOSEBLOCK 400
#define TOKEN	500
#define TEXT	600
#define DEFINE	700
#define TABLE	800
#define WORD	900
#define STRING	1000
#define FOREACH	1100
*/

int	brace_stack=0;
int	echo_stack=0;
int	if_stack=0;
int	foreach_stack=0;

/*text		([^{}\\$]|"\\{"|"\\}")+*/

/*<ECHOBLOCK>{openblock}	{ save_token(yytext); brace_stack++; return TEXT; }*/
/*
)|[A-Za-z]+\[([A-Za-z]+(([^\\][_\.])?[A-Za-z]+)*)
token	\$([A-Za-z]+(([^\\][_\.])?[A-Za-z]+)*|[A-Za-z]+\[([A-Za-z]+(([^\\][_\.])?[A-Za-z]+)*)
[A-Za-z_\[\]]+([^\\]\.[$A-Za-z_\[\]]+)*)
token	\$([A-Za-z_\[\]]+([^\\]\.[$A-Za-z_\[\]]+)*)
<ECHOBLOCK>"\\{"	|
<ECHOBLOCK>"\\}"	|
<ECHOBLOCK>"\\$"	|
<ECHOBLOCK>"\\"		|
<ECHOBLOCK>"\\_"	|
<ECHOBLOCK>"\\."	|
*/
%}

space	[ \t]+
newline	\n
word	[A-Za-z0-9_]+
string	\"[^\"\n]*\"
begincomment	"\/\*"
endcomment	"\*\/"
token	\$([A-Za-z]+(([^\\][_\.])?[A-Za-z]+)*|[A-Za-z]+(([^\\]_)?[A-Za-z]+)*\[([A-Za-z\[\]]+(([^\\][_\.])?[A-Za-z\[\]]+)*)*\])
text		[^\[\]\\$\n]+
openblock	"["[\n]?
closeblock	^[\t ]*"]"

%s ECHOBLOCK
%s COMMENTS

%%

<INITIAL>{space}	;
<INITIAL>{newline}	inc_lineno;

<INITIAL>{begincomment}	{ BEGIN COMMENTS; }

<COMMENTS>.		;

<COMMENTS>{newline}	inc_lineno;

<COMMENTS>{endcomment}	{ BEGIN INITIAL; }

<INITIAL>if		{ if_stack++; return IF; }

<INITIAL>"&&"		{ return AND; }

<INITIAL>"||"		{ return OR; }

<INITIAL>"="		{ return EQUAL; }

<INITIAL>"!"		{ return NOT; }

<INITIAL>{token}	{ save_token(yytext); return TOKEN; }

<INITIAL>{openblock}	{
				if(echo_stack) {
					BEGIN ECHOBLOCK; 
				} 
				if(strchr(yytext,'\n')) inc_lineno; 
				return OPENBLOCK; 
			}

<INITIAL>define		{ return DEFINE; }

<INITIAL>table		{ return TABLE; }

<INITIAL>else		{ return ELSE; }

<INITIAL>foreach	{ foreach_stack++; return FOREACH; }

<INITIAL>echo		{ 	echo_stack++; 
				return ECHO; 
			}

<INITIAL>{closeblock}	{ return CLOSEBLOCK; }

<INITIAL>{word}		{ save_token(yytext); return WORD; }

<INITIAL>{string}	{ 
				save_token(yytext+1); 
				*(strchr(yylval.string,'"'))='\0';
				return STRING; 
			}

<ECHOBLOCK>{token}	{ save_token(yytext); return TOKEN; }

<ECHOBLOCK>{text}	{ save_token(yytext); return TEXT; }

<ECHOBLOCK>\\.		{ save_token(yytext+1); return TEXT; }

<ECHOBLOCK>{space}	{ save_token(" "); return TEXT; }

<ECHOBLOCK>{newline}	{ save_token("\n"); inc_lineno; return TEXT; }

<ECHOBLOCK>{closeblock}	{
				if (brace_stack==0) {
					BEGIN INITIAL; 
					echo_stack--;
					return CLOSEBLOCK; 
				} else {
					brace_stack--;
					save_token(yytext);
					return TEXT;
				}
			}

<INITIAL>.			{ return yytext[0]; }

%%

int yywrap(void)
{
	return 1;
}

#ifdef STANDALONE

YYSTYPE yylval;

void main(void)
{
	int	i;

	while(i=yylex()) {
		printf("%s -> %d\n",yytext,i);
	}
}
#endif
