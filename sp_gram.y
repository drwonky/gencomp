%{

/* grammar code for sp_make */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*#include "sp_parse.h"*/
#include "symtab.h"
#include "globals.h"

extern char *yytext;
symtab_node	*node;


%}

%union {
	char *string;
}

%token PR_COMMENT PR_DEF PR_TYPE PR_INPUT PR_OUTPUT PR_RESULT PR_PARM PR_WORD PR_STRING 
%type <string> PR_WORD PR_STRING PR_COMMENT
%type <string> param_type parm_name value 

%%

sp:
	proc_def comment_list parm_list type_define parameter_list 
	;

proc_def:
	PR_DEF PR_WORD		{ /* set proc name */ 
					#if DEBUG == 9
					printf("Procedure name: %s\n",$2); 
					#endif

					if(!(node=create_symtab_node(symbol_table,"procedure",$2,SYMTAB_STR))) {
						yyerror("Error adding procedure name %s to symbol table",$2);
					} else {
						char *p=strdup($2),*q;
					
						q=p;
						while(*q) { *q=toupper(*q); q++; }

						if(!create_symtab_node(node,"name",$2,SYMTAB_STR)) {
							yyerror("Error adding procedure name %s to symbol table",$2);
						}
						if(!create_symtab_node(node,"upper",p,SYMTAB_STR)) {
							yyerror("Error adding procedure name %s to symbol table",$2);
						}

						free(p);
					}
				}
	;

comment_list:
	| comment_list comment
	;

comment: /* null for no comment */
	PR_COMMENT		{ 
					#if DEBUG == 9
				printf("Comment: %s\n",$1); 
					#endif
			}
	;

parm_list:
	| parm_list parm_def
	;

parm_def: /* null for no special parms */
	PR_PARM parm_name value	{ /* set parm value */ 
					#if DEBUG == 9
					printf("Parm name: %s\n",$2); 
					printf("value: %s\n",$3); 
					#endif

					if(!create_symtab_node(symbol_table,$2,$3,SYMTAB_STR)) {
						yyerror("Error adding parm %s=>%s to the symbol table",$2,$3);
					}
				}
	;

parm_name:
	PR_WORD
	;

value:
	PR_WORD			
	| PR_STRING		
	;

type_define:
	PR_TYPE PR_WORD	{ /* set type */ 
					#if DEBUG == 9
				printf("Type %s\n",$2); 
					#endif

				if(!create_symtab_node(symbol_table,"type",$2,SYMTAB_STR)) {
					yyerror("Error adding procedure type %s to symbol table",$2);
				}
			}
	;

parameter_list:
	| parameter_list parameter_def
	;

parameter_def:	
	PR_INPUT param_type PR_WORD { /* copy info */ 
					#if DEBUG == 9
					printf("Input parameter: %s -> %s\n",$3,$2); 
					#endif

					if(!pr_input) {
						if(!(pr_input=create_symtab_node(symbol_table,"input",NULL,SYMTAB_NUL))) {
							yyerror("Error creating input class\n");
						}
					}

					if(!(tmp_node=create_symtab_node(pr_input,$3,NULL,SYMTAB_NUL))) {
						yyerror("Error adding input parm %s to symbol table",$3);
					} else {
						if(!create_symtab_node(tmp_node,"name",$3,SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
						if(!create_symtab_node(tmp_node,"type",$2,SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
					}

					if(!(tmp_node=create_symtab_node(pr_list,$3,NULL,SYMTAB_NUL))) {
						yyerror("Error adding input parm %s to symbol table",$3);
					} else {
						if(!create_symtab_node(tmp_node,"name",$3,SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
						if(!create_symtab_node(tmp_node,"type",$2,SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
						if(!create_symtab_node(tmp_node,"class","input",SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
					}
				}
	| PR_OUTPUT param_type PR_WORD { /* copy info */ 
					#if DEBUG == 9
					printf("Output parameter: %s -> %s\n",$3,$2); 
					#endif

					if(!pr_output) {
						if(!(pr_output=create_symtab_node(symbol_table,"output",NULL,SYMTAB_NUL))) {
							yyerror("Error creating output class\n");
						}
					}

					if(!(tmp_node=create_symtab_node(pr_output,$3,NULL,SYMTAB_NUL))) {
						yyerror("Error adding output parm %s to symbol table",$3);
					} else {
						if(!create_symtab_node(tmp_node,"name",$3,SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
						if(!create_symtab_node(tmp_node,"type",$2,SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
					}

					if(!(tmp_node=create_symtab_node(pr_list,$3,NULL,SYMTAB_NUL))) {
						yyerror("Error adding output parm %s to symbol table",$3);
					} else {
						if(!create_symtab_node(tmp_node,"name",$3,SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
						if(!create_symtab_node(tmp_node,"type",$2,SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
						if(!create_symtab_node(tmp_node,"class","output",SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
					}
				}
	| PR_RESULT param_type PR_WORD { /* copy info */ 
					#if DEBUG == 9
					printf("Result parameter: %s -> %s\n",$3,$2); 
					#endif

					if(!pr_result) {
						if(!(pr_result=create_symtab_node(symbol_table,"result",NULL,SYMTAB_NUL))) {
							yyerror("Error creating result class\n");
						}
					}

					if(!(tmp_node=create_symtab_node(pr_result,$3,NULL,SYMTAB_NUL))) {
						yyerror("Error adding result parm %s to symbol table",$3);
					} else {
						if(!create_symtab_node(tmp_node,"name",$3,SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
						if(!create_symtab_node(tmp_node,"type",$2,SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
					}

					if(!(tmp_node=create_symtab_node(pr_list,$3,NULL,SYMTAB_NUL))) {
						yyerror("Error adding result parm %s to symbol table",$3);
					} else {
						if(!create_symtab_node(tmp_node,"name",$3,SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
						if(!create_symtab_node(tmp_node,"type",$2,SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
						if(!create_symtab_node(tmp_node,"class","result",SYMTAB_STR)) {
							yyerror("Error adding attribute type=>%s for parm %s to symbol table",$2,$3);
						}
					}
				}
	;

param_type:
	PR_WORD	
	;


%%

/*
#define prerror printf
int prerror(char *msg)
{
	printf("%s\n",msg);
}
*/

/*
int main(void)
{
	FILE	*prin=stdin;

	while(!feof(prin)) {
		prparse();
	}
}
*/
