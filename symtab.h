#ifndef SYMTAB_H
#define SYMTAB_H

#ifndef BOOLEAN
#define BOOLEAN int
#define TRUE 1
#define FALSE 0
#endif

typedef struct symtab_node {

	char	*name;
	int	type;

	union {
		char *string;
		int number;
	} val;
	
	struct symtab_node	*child;
	struct symtab_node	*peer;
	struct symtab_node	*last_peer;

} symtab_node;

#define SYMTAB_NUL 0
#define SYMTAB_STR 1
#define SYMTAB_INT 2

symtab_node	*symbol_table;	/* head node */

symtab_node *new_symtab_node(void);
void free_symtab_node(symtab_node *node);
symtab_node *create_symtab_node(symtab_node *parent, char *name, void *value, int type);
symtab_node *lookup_symtab_node(symtab_node *parent, char *name);
symtab_node *resolve_symtab_reference(symtab_node *parent, char *name);
BOOLEAN get_symtab_value(symtab_node *parent, char *name, void **value, int *type);
BOOLEAN update_symtab_value(symtab_node *parent, char *name, void *value, int type);

#endif
