#include "stack.h"
#include "machine.h"
#include "symtab.h"
#include "opcodes.h"
#include "datatypes.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct label {
	char		*name;
	Instruction	*offset;

	struct label	*next;
} Label;

Label	*label_list;

/* nice macro for getting the parms of an opcode */
#define fetch_arg(ip,parm) do{ \
	i=((int *)ip+sizeof(Instruction)/sizeof(int)); \
	p=(char *)ip+sizeof(Instruction)+sizeof(int); \
	parm.type=*i; \
	parm.parm=(char *)p; \
} while(0)

BOOLEAN add_label(char *name, Instruction *offset)
{
	Label	*label;

	#if DEBUG == 7
	fprintf(stderr,"After Label offset: %p\n",offset);
	#endif
	label=malloc(sizeof(Label));

	if(!label) return FALSE;

	label->name=strdup(name);
	label->offset=offset;

	label->next=label_list;
	label_list=label;

	return TRUE;
}

Instruction *find_label(char *name)
{
	Label	*label;

	label=label_list;

	while(label) {
		if(strcmp(label->name,name)==0) return label->offset;
		label=label->next;
	}

	return NULL;
}

BOOLEAN init_program(void)
{
	program=malloc(sizeof(Program));

	if(!program) return FALSE;

	program->prog=malloc(DEF_PROG_SIZE);

	if(!program->prog) return FALSE;

	program->prog_ip=program->prog;

	program->prog_size=DEF_PROG_SIZE;
	program->prog_cur_size=0;

	return TRUE;
}

BOOLEAN write_output(int fd, char *data, int size)
{
	char	*p;
	int	n,total;

	#if DEBUG == 7
	fprintf(stderr,"%d:Data: '%s'\n",fd,data);
	#endif

	total=0;
	p=data;
	n=0;
	while(size-total > 0) {
	#if DEBUG == 7
		fprintf(stderr,"writing: '%s'\n",p);
	#endif
		n=write(fd,p,size-total);
		if(n < 0) break;
		p+=n;
		total+=n;
	}

	if(n < 0) return FALSE;

	return TRUE;
}

BOOLEAN run_program(void)
{
	Instruction	*ip,*new_ip;
	Opparm		parm;
	symtab_node	*node;
	int		scan=0;
	BOOLEAN		bool;
	char		*scan_label=NULL;
	char		*p;
	int		*i;

	stack=NULL;
	ip=(Instruction *)program->prog;
	label_list=NULL;

	while(ip < (Instruction *)program->prog + program->prog_cur_size) {
	#if DEBUG == 7
		fprintf(stderr,"opcode: %d (%s)\n",ip->op,opcodes_txt[ip->op]);
		fprintf(stderr,"opcode len: %d\n",ip->op_len);
		fprintf(stderr,"IP: 0x%08X\n",ip-(Instruction *)program->prog);
	#endif
		if(!scan || ip->op == op_label || ip->op == op_halt) {
			switch(ip->op) {
				case op_push:
					fetch_arg(ip,parm);
	
					push_stack(parm.type,parm.parm);
					break;
				case op_lookup:
					fetch_arg(ip,parm);
	
					node=resolve_symtab_reference(symbol_table,(char *)parm.parm);
					if(!node) {
						push_stack(ST_STRING,"NULL");
					} else {
						switch(node->type) {
							case SYMTAB_NUL:
								push_stack(ST_STRING,"EXISTS");
								break;
							case SYMTAB_STR:
								push_stack(ST_STRING,node->val.string);
								break;
							case SYMTAB_INT:
								push_stack(ST_INT,&node->val.number);
								break;
						}
					}

					break;
				case op_pop:
					pop_stack();
					break;
				case op_store: /* not used */
					fetch_arg(ip,parm);
	
					break;
				case op_label: 
					fetch_arg(ip,parm);
	
	#if DEBUG == 7
					fprintf(stderr,"Label: %p\n",ip);
	#endif
					if(!add_label((char *)parm.parm,(Instruction *)((char *)ip+ip->op_len))) 
						return FALSE;

					if(scan) {
						if(strcmp((char *)parm.parm,scan_label)==0) 
							scan=0;
					}
					break;
				case op_jump:
					fetch_arg(ip,parm);
	
	#if DEBUG == 7
					fprintf(stderr,"Find label: %s\n",(char *)parm.parm);
	#endif
					if(!(new_ip=find_label((char *)parm.parm))) {
						scan=1;
						scan_label=(char *)parm.parm;
	#if DEBUG == 7
						fprintf(stderr,"Scanning for label: %s\n",scan_label);
	#endif
					} else {
						ip=new_ip;
	#if DEBUG == 7
						fprintf(stderr,"Found label: %s -> %p\n",scan_label,new_ip);
	#endif
						continue;
					}
	
	#if DEBUG == 7
					fprintf(stderr,"after jump: %p\n",ip);
	#endif
					break;
				case op_jumpz:
					fetch_arg(ip,parm);
	
					bool=0;
					if(stack->type==ST_BOOL) {
						if(!stack->val.st_bool) {
							bool=1;
						} else {
							bool=0;
						}
					} else {
						if(stack->type==ST_SYMTAB) {
							if(stack->val.st_symtab->type != SYMTAB_NUL)
								bool=1;
							else	bool=0;
						} else	bool=1; /* it's a unary argument */
					}

					if(bool) {
	#if DEBUG == 7
						fprintf(stderr,"Find label: %s\n",(char *)parm.parm);
	#endif
						if(!(new_ip=find_label((char *)parm.parm))) {
							scan=1;
							scan_label=(char *)parm.parm;
	#if DEBUG == 7
							fprintf(stderr,"Scanning for label: %s\n",scan_label);
	#endif
						} else {
							ip=new_ip;
	#if DEBUG == 7
							fprintf(stderr,"Found label: %s -> %p\n",scan_label,new_ip);
	#endif
							pop_stack();
							continue;
						}
					} 

	
	#if DEBUG == 7
					fprintf(stderr,"after jump: %p\n",ip);
	#endif
					pop_stack();
					break;
				case op_jumpnz:
					fetch_arg(ip,parm);
	
					bool=0;
					if(stack->type==ST_BOOL) {
						if(!stack->val.st_bool) {
							bool=1;
						} else {
							bool=0;
						}
					} else {
						if(stack->type==ST_SYMTAB) {
							if(stack->val.st_symtab->type != SYMTAB_NUL)
								bool=1;
							else	bool=0;
						} else	bool=1; /* it's a unary argument */
					}

					if(!bool) {
	#if DEBUG == 7
						fprintf(stderr,"Find label: %s\n",(char *)parm.parm);
	#endif
						if(!(new_ip=find_label((char *)parm.parm))) {
							scan=1;
							scan_label=(char *)parm.parm;
	#if DEBUG == 7
							fprintf(stderr,"Scanning for label: %s\n",scan_label);
	#endif
						} else {
							ip=new_ip;
							pop_stack();
							continue;
						}
					} 
	
					pop_stack();
	#if DEBUG == 7
					fprintf(stderr,"after jump: %p\n",ip);
	#endif
					break;
				case op_halt:
					return TRUE;

				case op_not:
					if(stack->type!=ST_BOOL) 
						return FALSE;

					stack->val.st_bool=!stack->val.st_bool;

					break;
				case op_cmpnot:
					bool=compare_stack(stack,stack->next);
					pop_stack();
					pop_stack();

					if(!bool) {
						push_stack(ST_BOOL,"\1");
					} else {
						push_stack(ST_BOOL,"\0");
					}
					break;
				case op_cmp:
					bool=compare_stack(stack,stack->next);
					pop_stack();
					pop_stack();

					if(bool) {
						push_stack(ST_BOOL,"\1");
					} else {
						push_stack(ST_BOOL,"\0");
					}
					break;
				case op_and:
					bool=and_stack(stack,stack->next);
					pop_stack();
					pop_stack();

					if(bool) {
						push_stack(ST_BOOL,"\1");
					} else {
						push_stack(ST_BOOL,"\0");
					}
					break;
				case op_or:
					bool=or_stack(stack,stack->next);
					pop_stack();
					pop_stack();

					if(bool) {
						push_stack(ST_BOOL,"\1");
					} else {
						push_stack(ST_BOOL,"\0");
					}
					break;
				case op_write:
					fetch_arg(ip,parm);

					if(stack->type != ST_STRING) {
	#if DEBUG == 7
						fprintf(stderr,"Stack type: %d\n",stack->type);
	#endif
						return FALSE;
					}

					node=(symtab_node *)*(int *)parm.parm;
					write_output(node->val.number,stack->val.st_string,strlen(stack->val.st_string));
					pop_stack();

					break;
				case op_next:
					fetch_arg(ip,parm);

					if(parm.type != ST_STRING) 
						return FALSE;

					/* get loop parm address */
					node=lookup_symtab_node(symbol_table,(char *)parm.parm);

					if(!node) return FALSE;

					/* use loop parm child as tmp var */
					node->child=stack->val.st_symtab->peer;
					pop_stack();	/* take old pointer off stack */

					/* is this the end of the class */
					if(node->child) { 
						/* no, update parm, push true */
						push_stack(ST_SYMTAB,&node->child);

						/* this is the child of the current element in the class */
						node->child=node->child->child;
						push_stack(ST_BOOL,"\1");
					} else {
						/* yes, push false */
						push_stack(ST_BOOL,"\0");
					}
					
					break;
				case op_alias:
					fetch_arg(ip,parm);

					if(parm.type != ST_STRING) 
						return FALSE;

					/* get loop parm address */
					node=lookup_symtab_node(symbol_table,(char *)parm.parm);

					if(!node) return FALSE;

					/* use loop parm child as tmp var */
					/* stack contains parent class, set parm to first element in class */
					node->child=stack->val.st_symtab->child;
					pop_stack();	/* take old pointer off stack */

					/* is this the end of the class */
					if(node->child) { 
						/* no, update parm, push true */
						push_stack(ST_SYMTAB,&node->child);
						node->child=node->child->child;
					} 
					
					break;
			}
		}	

	#if DEBUG == 7
		fprintf(stderr,"before: %p\n",ip);
	#endif
		ip+=(ip->op_len/sizeof(Instruction));
	#if DEBUG == 7
		fprintf(stderr,"after: %p\n",ip);
	#endif
	}

	return TRUE;
}
