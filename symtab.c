#include <stdlib.h>
#include <string.h>
#include "symtab.h"

symtab_node *new_symtab_node(void)
{
	symtab_node	*node;

	node=malloc(sizeof(symtab_node));

	if(!node) return NULL;

	node->name=NULL;
	node->type=SYMTAB_NUL;

	node->val.string=NULL;

	node->child=NULL;
	node->last_peer=NULL;
	node->peer=NULL;

	return node;
}

void free_symtab_node(symtab_node *node)
{
	if(!node) return;

	if(node->name) free(node->name);

	if(node->type==SYMTAB_STR) {
		if(node->val.string) free(node->val.string);
	}

	free(node);
}

symtab_node *create_symtab_node(symtab_node *parent, char *name, void *value, int type)
{
	symtab_node	*node;

	node=new_symtab_node();

	if(!node) return NULL;

	if(*name=='\0') {
		yyerror("Null node name passed to create_symtab_node");
		return NULL;
	}

	node->name=strdup(name);

	node->type=type;

	if(type==SYMTAB_STR) {
		node->val.string=strdup((char *)value);
	} else if(type==SYMTAB_INT) {
		node->val.number=*((int *)value);
	}

	/* ghastly tail pointers, grr, LIFOs are so much more elegant :) */
	if(!parent->child) {	
		parent->child=node;
		node->last_peer=node;
	} else {
		parent->child->last_peer->peer=node;
		parent->child->last_peer=node;
	}

	return node;
}

symtab_node *lookup_symtab_node(symtab_node *parent, char *name)
{
	symtab_node	*node;

	node=parent->child;

	while(node) {
		if(strcmp(node->name,name)==0) {
			return node;
		}
		node=node->peer;
	}

	return NULL;
}

/* 

This function implements a recursive resolution of a variable reference.
The variables formats accepted are:

class[base_node.attribute]  #find the value in class that matches node.attrib
base_node.attribute	#get node.attribute value
class.base_node.attribute	#get class.node.attrib value

Where:

class is the parent of the base_node, in a tree it is higher up.  base_node
contains data or is a parent to attributes (or both).  attributes are simply
sub nodes of the base node, specific to that base node.
*/
symtab_node *resolve_symtab_reference(symtab_node *parent, char *name)
{
	char		*p,*q;
	char		tmp_name[1024];
	char		tmp_indx[1024];
	symtab_node	*node,*class;

	/* check for array type reference */
	p=strchr(name,'[');

	if(p) {		/* array reference */

		/* too long? */
		if(p-name > 1023) return NULL;	

		/* copy everything up to '[' */
		strncpy(tmp_name,name,p-name);
		tmp_name[p-name]='\0';

		/* find ending bracket */
		q=strchr(p,']');

		/* skip open bracket */
		p++;

		/* if there's an ending bracket */
		if(q) {

			/* size check */
			if(q-p > 1023) return NULL;

			/* copy up to ending bracket */
			strncpy(tmp_indx,p,q-p);
			tmp_indx[q-p]='\0';

		} else return NULL;

		/* now we find the node for the base name of class */
		if(class=lookup_symtab_node(parent,tmp_name)) 
			/* recursive resolution of node */
			node = resolve_symtab_reference(class,tmp_indx);
		else return NULL;

		if(!node) return NULL;
		/* if resultant value of array index is not text, barf */
		if(node->type != SYMTAB_STR) return NULL;

		/* find element of class who's name = node's value */
		return lookup_symtab_node(class,node->val.string);

	} else {

		/* it's not an class, is it an attribute reference? */
		p=strchr(name,'.');

		if(p) {		/* attribute reference */

			/* too big? */
			if(p-name > 1023) return NULL;

			/* nope, copy the base node name */
			strncpy(tmp_name,name,p-name);
			tmp_name[p-name]='\0';

			/* skip the period */
			p++;

			/* find the base node */
			if(node=lookup_symtab_node(symbol_table,tmp_name)) 
				/* look for the attribute of the base node */
				return resolve_symtab_reference(node,p);
			else return NULL;
		} else {	/* it's not an class or attribute reference, its just a node */
			return lookup_symtab_node(parent,name);
		}
	}
}

/*
BOOLEAN get_symtab_value(symtab_node *parent, char *name, void *value, int *type)
{
	symtab_node	*node;

	node=lookup_symtab_node(parent,name);

	if(!node) return FALSE;

	*type=node->type;

	if(node->type==SYMTAB_STR) {
		(char *)value=node->val.string;
	} else if(node->type==SYMTAB_INT) {
		(int)*(int *)value=&node->val.number;
	}

	return TRUE;
}
*/

BOOLEAN update_symtab_value(symtab_node *parent, char *name, void *value, int type)
{
	symtab_node	*node;

	node=lookup_symtab_node(parent,name);

	if(!node) {
		if(!create_symtab_node(parent,name,value,type))
			return FALSE;
	} else {
		if(node->type==SYMTAB_STR) {
			free(node->val.string);
			node->val.string=strdup((char *)value);
		} else if(node->type==SYMTAB_INT) {
			node->val.number=*((int *)value);
		}

		node->type=type;
	}

	return TRUE;
}
