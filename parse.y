/*
 * $Id$
 *
 * Gencomp grammar parsing code, written by Perry Harrington
 *
 * $Log$
 * Revision 1.4  1998/10/03 01:04:58  pedward
 * removed parse.h from the include list
 *
 * Revision 1.3  1998/09/24 00:22:19  pedward
 * fixed foreach parsing so that if it doesn't find the class that it's looping in, it doesn't
 * generate the code.  Was causing a core because of another fix.
 *
*/

%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "globals.h"
#include "code.h"

extern char *yytext;
extern int lineno;

/* for generating random labels */
int	iflabel_cnt=0;
int	forlabel_cnt=0;
int	if_level=0;		/* used with the label */
char	iflabel_tmp[15];	/* if & foreach have different needs */
char	forlabel_tmp[15];
int	iflabel_ref[17];

symtab_node	*current_parent_class=NULL;
symtab_node	*current_descriptor=NULL;
symtab_node	*node=NULL;

%}

%union {
	char *string;
}

%token IF ECHO OPENBLOCK CLOSEBLOCK DEFINE TABLE FOREACH ELSE
%token EQUAL NOT OR AND
%token <string> TEXT WORD STRING TOKEN
%type <string> define_parms expression echo_data value

%left OR
%left AND 
%left '(' ')' NOT EQUAL

%%

statement_list: 	
	| statement_list statement 
	;

statement: 	define_statement
	|	if_statement
	|	foreach_statement
	|	echo_statement
	;

define_statement:
	DEFINE TABLE WORD { /* add to symbol table */ 
		#if DEBUG == 9
			printf("Defined symbol: %s\n",$3); 
		#endif

		if(!(current_parent_class=create_symtab_node(symbol_table,$3,NULL,SYMTAB_NUL))) {
			yyerror("Error creating symbol table entry: %s\n",$3);
		}


	}
	OPENBLOCK define_list CLOSEBLOCK
	;

define_list:
	| define_list define_parms
	;

define_parms:
	value value { /* add item to table */

			if(current_parent_class) {
				if(!create_symtab_node(current_parent_class,$1,$2,SYMTAB_STR)) {
					yyerror("Couldn't add %s=>%s to %s class\n",$1,$2,current_parent_class->name);
				}
			} else {
				yyerror("Trying to add an item to a class, but no class is defined!");
			}

			#if DEBUG == 9
				printf("name: %s value: %s\n",$1, $2); 
			#endif
			
		}
	;

value:		WORD
	|	STRING
	;

statement_block:
	OPENBLOCK statement_list CLOSEBLOCK

if_statement:
	IF '(' expression ')' { /* output compare */ 
		#if DEBUG == 9
			printf("If statement\n");
		#endif

		iflabel_ref[if_level]=iflabel_cnt++;
		sprintf(iflabel_tmp,"iflabel%d",iflabel_ref[if_level]);
		if_level++;

		generate((Program *)program, op_jumpz, ST_STRING, iflabel_tmp);
	}
	statement_block { /* output jump label */
		#if DEBUG >= 8
			printf("Generate label for if\n");
		#endif

		if_level--;
		sprintf(iflabel_tmp,"elselabel%d",iflabel_ref[if_level]);
		generate((Program *)program, op_jump, ST_STRING, iflabel_tmp);
		sprintf(iflabel_tmp,"iflabel%d",iflabel_ref[if_level]);
		generate((Program *)program, op_label, ST_STRING, iflabel_tmp);
	}
	else_statement 
	;

else_statement: {
			sprintf(iflabel_tmp,"elselabel%d",iflabel_ref[if_level]);
			generate((Program *)program, op_label, ST_STRING, iflabel_tmp);
		}
	| ELSE statement_block { /* output jump label */
		#if DEBUG >= 8
			printf("Generate label for if\n");
		#endif

		sprintf(iflabel_tmp,"elselabel%d",iflabel_ref[if_level]);
		generate((Program *)program, op_label, ST_STRING, iflabel_tmp);
	}
	;

expression:	expression EQUAL EQUAL expression { 
			#if DEBUG >= 8
				printf("Generate 1 == 3\n");
			#endif

			generate((Program *)program, op_cmp);
		}

	|	expression NOT EQUAL expression {
			#if DEBUG >= 8
				printf("Generate 1 != 3\n");
			#endif

			generate((Program *)program, op_cmpnot);
		}
	|	NOT '(' expression ')' {
			#if DEBUG >= 8
				printf("Generate ! 2\n");
			#endif

			generate((Program *)program, op_not);
		}
	|	expression OR expression {
			#if DEBUG >= 8
				printf("Generate 1 || 3\n");
			#endif

			generate((Program *)program, op_or);
		}
	|	expression AND expression {
			#if DEBUG >= 8
				printf("Generate 1 && 3\n");
			#endif

			generate((Program *)program, op_and);
		}
	| '(' expression ')' { $$=$2; }
	|	TOKEN { /* push item */
			#if DEBUG == 9
				printf("found %s in expression\n",$1); 
			#endif

			generate((Program *)program, op_lookup, ST_STRING, $1+1);

		}
	|	WORD { /* push item */
			#if DEBUG == 9
				printf("found %s in expression\n",$1); 
			#endif

			if(node=resolve_symtab_reference(symbol_table,$1))
				generate((Program *)program, op_push, ST_SYMTAB, node);
			else	generate((Program *)program, op_push, ST_STRING, "NULL");

		}
	|	STRING { /* push item */
			#if DEBUG == 9
				printf("found %s in expression\n",$1); 
			#endif

			generate((Program *)program, op_push, ST_STRING, $1);

		}
	;

foreach_statement:
	FOREACH WORD WORD { /* add parm to table */
		#if DEBUG == 9
			printf("Foreach variable: '%s' class: '%s'\n",$2,$3); 
		#endif

		if((current_parent_class=node=lookup_symtab_node(symbol_table,$3))) {
			if(node->child) {

				if(!create_symtab_node(symbol_table,$2,NULL,SYMTAB_NUL)) {
					yyerror("Can't add %s foreach variable to symbol table");
				}

				#if DEBUG >= 8
					printf("Push parm on stack\n");		
					printf("Alias parm\n");		
					printf("output label\n");
				#endif

				/* push parm */
				generate((Program *)program, op_push, ST_SYMTAB, lookup_symtab_node(symbol_table,$3));

				/* alias parm's child to class' first entry's child */
				/* pop's previous parm */
				generate((Program *)program, op_alias, ST_STRING, $2);

				sprintf(forlabel_tmp,"forlabel%d",forlabel_cnt++);

				generate((Program *)program, op_label, ST_STRING, forlabel_tmp);

			} else {
				iflabel_ref[if_level]=iflabel_cnt++;
				sprintf(iflabel_tmp,"iflabel%d",iflabel_ref[if_level]);
				if_level++;

				generate((Program *)program, op_jump, ST_STRING, iflabel_tmp);
			}
		}
	}
	statement_block { /* gen loop code */
		#if DEBUG >= 8
			printf("Next op\n");
			printf("Generate if not 0 goto label\n");
		#endif

		if(current_parent_class) {
			if(current_parent_class->child) {
				/* iterative aliasing of parm to members in class */
				/* if end of class push 0 else push 1 */
				generate((Program *)program, op_next,ST_STRING,$2);

				/* jump if previous was true */
				/* check top of stack and pop one */
				generate((Program *)program, op_jumpnz, ST_STRING, forlabel_tmp);
			} else {
				if_level--;

				sprintf(iflabel_tmp,"iflabel%d",iflabel_ref[if_level]);
				generate((Program *)program, op_label, ST_STRING, iflabel_tmp);
			}
		}
	}
	;

echo_statement:
	ECHO WORD { /* push descriptor name */
		#if DEBUG == 9
			printf("Echo descriptor name: '%s'\n",$2); 
		#endif

		#if DEBUG >= 8
			printf("Set current fd to descriptor lookup value\n");
		#endif

		if(!(current_descriptor = lookup_symtab_node(symbol_table,$2))) {
			yyerror("'%s' is not a valid descriptor",$2);
		}

	}
	OPENBLOCK echo_list CLOSEBLOCK
	;

echo_list:
	| echo_list echo_data 
	;

echo_data:	TEXT { 
			#if DEBUG == 9
				printf("text: %s\n",$1); 
			#endif

			#if DEBUG >= 8
				printf("Push text onto stack\n");
				printf("Generate write output\n");
			#endif

			generate((Program *)program, op_push, ST_STRING, $1);
			generate((Program *)program, op_write, ST_SYMTAB, current_descriptor);

		}
	|	TOKEN { /* generate lookup and echo code */
			#if DEBUG == 9
				printf("Token found: %s\n",$1); 
			#endif

			#if DEBUG >= 8
				printf("Generate op lookup\n");
				printf("Generate write outpute\n");
			#endif

			/* resolve token reference */
			/* resolve and push literal onto stack */
			generate((Program *)program, op_lookup, ST_STRING, $1+1);
			/* write result to file descriptor */
			/* pop and write */
			generate((Program *)program, op_write, ST_SYMTAB, current_descriptor);

		}
	;

%%

