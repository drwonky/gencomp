#ifndef STACK_H
#define STACK_H

#include "symtab.h"
#include "datatypes.h"

/* stack data types */
#define	ST_STRING	1
#define	ST_INT		2
#define	ST_BOOL		3
#define	ST_SYMTAB	4

typedef struct stack {
	int	type;		/* data type */

	union {
		char		*st_string;
		int		st_int;
		char		st_bool;
		symtab_node	*st_symtab;
	} val;

	struct stack	*next;	/* next item on stack */
} Stack;

BOOLEAN push_stack(int type, void *value);
void pop_stack(void);
BOOLEAN compare_stack(Stack *src, Stack *dst);
BOOLEAN or_stack(Stack *src, Stack *dst);
BOOLEAN and_stack(Stack *src, Stack *dst);

#endif
