#include "stack.h"
#include "datatypes.h"
#include "globals.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

BOOLEAN push_stack(int type, void *value)
{
	Stack	*node;

	node=malloc(sizeof(Stack));

	if(!node) return FALSE;

	node->type=type;

	#if DEBUG == 6
	fprintf(stderr,"Push type: %d\n",type);
	#endif

	switch(type) {
		case ST_STRING:
			node->val.st_string=strdup((char *)value);
	#if DEBUG == 6
			fprintf(stderr,"Stack value: %s\n",(char *)value);
	#endif
			break;
		case ST_INT:
			node->val.st_int=*(int *)value;
			break;
		case ST_BOOL:
			node->val.st_bool=(*(char *)value == 0 ? 0 : 1);
			break;
		case ST_SYMTAB:
			node->val.st_symtab=(symtab_node *)(*((int *)value));
			break;
	}

	node->next=stack;
	stack=node;

	return TRUE;
}

void pop_stack(void)
{
	Stack	*node;

	if(stack->type==ST_STRING) {
		free(stack->val.st_string);
	}

	node=stack;
	stack=node->next;

	free(node);
}

BOOLEAN compare_stack(Stack *src, Stack *dst)
{
	char	*a,*b;
	char	tmp_numa[15], tmp_numb[15];

	switch(src->type) {
		case ST_SYMTAB:
			switch(src->val.st_symtab->type) {
				case SYMTAB_NUL:
					sprintf(tmp_numa,"EXISTS");
					a=tmp_numa;
					break;
				case SYMTAB_STR:
					a=src->val.st_symtab->val.string;
					break;
				case SYMTAB_INT:
					sprintf(tmp_numa,"%d",src->val.st_symtab->val.number);
					a=tmp_numa;
					break;
			}
			break;
		case ST_INT:
			sprintf(tmp_numa,"%d",src->val.st_int);
			a=tmp_numa;
			break;
		case ST_BOOL:
			sprintf(tmp_numa,"%s",src->val.st_bool ? "1" : "0");
			a=tmp_numa;
			break;
		case ST_STRING:
			a=src->val.st_string;
			break;
	}

	switch(dst->type) {
		case ST_SYMTAB:
			switch(dst->val.st_symtab->type) {
				case SYMTAB_NUL:
					sprintf(tmp_numb,"EXISTS");
					b=tmp_numb;
					break;
				case SYMTAB_STR:
					b=dst->val.st_symtab->val.string;
					break;
				case SYMTAB_INT:
					sprintf(tmp_numb,"%d",dst->val.st_symtab->val.number);
					b=tmp_numb;
					break;
			}
			break;
		case ST_INT:
			sprintf(tmp_numb,"%d",dst->val.st_int);
			b=tmp_numb;
			break;
		case ST_BOOL:
			sprintf(tmp_numb,"%s",dst->val.st_bool ? "1" : "0");
			b=tmp_numb;
			break;
		case ST_STRING:
			b=dst->val.st_string;
			break;
	}

	if(strcmp(a,b)==0)
		return TRUE;

	return FALSE;
}

BOOLEAN or_stack(Stack *src, Stack *dst)
{
	char	*a,*b;
	char	tmp_numa[15], tmp_numb[15];

	switch(src->type) {
		case ST_SYMTAB:
			a=src->val.st_symtab->val.string;
			if(a && *a) return TRUE;
			break;
		case ST_INT:
			if(src->val.st_int) return TRUE;
			break;
		case ST_BOOL:
			if(src->val.st_bool) return TRUE;
			break;
		case ST_STRING:
			a=src->val.st_string;
			if(a && *a) return TRUE;
			break;
	}

	switch(dst->type) {
		case ST_SYMTAB:
			b=dst->val.st_symtab->val.string;
			if(b && *b) return TRUE;
			break;
		case ST_INT:
			if(dst->val.st_int) return TRUE;
			break;
		case ST_BOOL:
			if(dst->val.st_bool) return TRUE;
			break;
		case ST_STRING:
			b=dst->val.st_string;
			if(b && *b) return TRUE;
			break;
	}

	return FALSE;
}

BOOLEAN and_stack(Stack *src, Stack *dst)
{
	char	*a,*b;
	char	tmp_numa[15], tmp_numb[15];

	switch(src->type) {
		case ST_SYMTAB:
			a=src->val.st_symtab->val.string;
			if(a && !*a) return FALSE;
			break;
		case ST_INT:
			if(!src->val.st_int) return FALSE;
			break;
		case ST_BOOL:
			if(!src->val.st_bool) return FALSE;
			break;
		case ST_STRING:
			a=src->val.st_string;
			if(a && !*a) return FALSE;
			break;
	}

	switch(dst->type) {
		case ST_SYMTAB:
			b=dst->val.st_symtab->val.string;
			if(b && !*b) return FALSE;
			break;
		case ST_INT:
			if(!dst->val.st_int) return FALSE;
			break;
		case ST_BOOL:
			if(!dst->val.st_bool) return FALSE;
			break;
		case ST_STRING:
			b=dst->val.st_string;
			if(b && !*b) return FALSE;
			break;
	}

	return TRUE;
}
