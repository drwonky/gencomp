#ifndef GLOBALS_H
#define GLOBALS_H

#include "symtab.h"
#include "stack.h"
#include "machine.h"

symtab_node	*symbol_table;

symtab_node	*pr_input;	/* input parm class */
symtab_node	*pr_output;	/* output parm class */
symtab_node	*pr_result;	/* result set class */
symtab_node	*pr_list;	/* all parameters */

symtab_node	*tmp_node;	/* for temp use in parser */

int		lineno;

Program		*program;
Stack		*stack;

#endif
